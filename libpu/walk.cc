
/*
 * Copyright 2004-2006 Jeff Garzik <jgarzik@pobox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1			/* for O_DIRECTORY (and more?) */
#define _DARWIN_C_SOURCE 1		/* for O_DIRECTORY */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <libpu.h>


static int walk_dirent(struct walker *w, int dirfd,
		       const char *dirn, const char *basen);


static int walk_cmdline_fd(struct walker *w, const char *fn, const struct stat *st)
{
	int fd, err;

	if ((fn == NULL) ||
	    ((w->flags & WF_STDIN_DASH) && (!strcmp(fn, "-"))))
		return w->cmdline_fd(w, _("(standard input)"), STDIN_FILENO);

	fd = open(fn, O_RDONLY);
	if (fd < 0) {
		perror(fn);
		w->exit_status = EXIT_FAILURE;
		return 0;
	}

	err = w->cmdline_fd(w, fn, fd);

	if (close(fd) < 0) {
		perror(fn);
		w->exit_status = EXIT_FAILURE;
	}

	return err;
}

static int walk_cmdline_file(struct walker *w, const char *fn, const struct stat *st)
{
	const char *pr_fn;
	FILE *f;
	int rc;

	if ((fn == NULL) ||
	    ((w->flags & WF_STDIN_DASH) && (!strcmp(fn, "-")))) {
		pr_fn = _("(standard input)");
		f = stdin;
	} else {
		pr_fn = fn;
		if (ro_file_open(&f, fn)) {
			w->exit_status = EXIT_FAILURE;
			return 0;
		}
	}

	rc = w->cmdline_file(w, pr_fn, f);

	if (ferror(f)) {
		perror(pr_fn);
		w->exit_status = EXIT_FAILURE;
	}

	if (fn != NULL)
		if (fclose(f)) {
			perror(fn);
			w->exit_status = EXIT_FAILURE;
		}

	return rc;
}

static int walk_iterate(struct walker *w, int old_dirfd,
			const char *dirn, const char *basen)
{
	DIR *dir;
	char *alloc_fn = NULL;
	const char *fn;
	int dirfd, flags, rc = 0;
	unsigned int testbits;
	struct dirent *de, *entry;
	int have_path = (dirn == NULL);

	if (have_path)
		fn = basen;
	else {
		alloc_fn = strpathcat(dirn, basen);
		fn = alloc_fn;
	}

	entry = (struct dirent *) xmalloc(sizeof(struct dirent) + NAME_MAX + 1);

	testbits = WF_FOLLOW_LINK;
	if (have_path)
		testbits |= WF_FOLLOW_LINK_CMDLINE;

	flags = O_DIRECTORY;
	if ((w->flags & testbits) == 0)
		flags |= O_NOFOLLOW;

	dirfd = open(basen, flags);
	if (dirfd < 0) {
		perror(fn);
		goto out;
	}

	if (fchdir(dirfd) < 0) {
		perror(fn);
		goto out_fd;
	}

	dir = opendir(".");
	if (!dir) {
		perror(fn);
		goto out_chdir;
	}

	while (1) {
		int drc;

		de = NULL;
		if (readdir_r(dir, entry, &de)) {
			perror(fn);
			break;
		}
		if (de != entry)
			break;

		drc = walk_dirent(w, dirfd, fn, de->d_name);
		if (drc) {
			rc = drc;
			break;
		}
	}

	if (closedir(dir) < 0)
		perror(fn);

out_chdir:
	if (fchdir(old_dirfd) < 0)
		perror(have_path ? "." : dirn);
out_fd:
	if (close(dirfd) < 0)
		perror(fn);
out:
	free(entry);
	if (!have_path)
		free(alloc_fn);
	return rc;
}

static int walk_dirent(struct walker *w, int dirfd,
		       const char *dirn, const char *basen)
{
	struct stat st;
	int have_stat = 0;
	int rc;

	if (w->flags & WF_STAT) {
		if (w->flags & WF_FOLLOW_LINK)
			rc = stat(basen, &st);
		else
			rc = lstat(basen, &st);
		have_stat = 1;
	}

	rc = w->dirent(w, dirn, basen, have_stat ? &st : NULL);
	switch (rc) {
	case RC_OK:		return 0;
	case RC_STOP_WALK:	return 1;
	case RC_RECURSE:	return walk_iterate(w, dirfd, dirn, basen);
	}

	/* should never occur */
	assert(0);
	return 1;
}

static int walk_entry(struct walker *w, const char *arg)
{
	struct stat st;
	int have_stat = 0;
	int rc;

	if (w->flags & WF_STAT) {
		if (w->flags & (WF_FOLLOW_LINK | WF_FOLLOW_LINK_CMDLINE))
			rc = stat(arg, &st);
		else
			rc = lstat(arg, &st);
		have_stat = 1;
	}

	if (w->cmdline_file)
		rc = walk_cmdline_file(w, arg, have_stat ? &st : NULL);
	else if (w->cmdline_fd)
		rc = walk_cmdline_fd(w, arg, have_stat ? &st : NULL);
	else
		rc = w->cmdline_arg(w, arg, have_stat ? &st : NULL);
	switch (rc) {
	case RC_OK:		return 0;
	case RC_STOP_WALK:	return 1;
	case RC_RECURSE:	return walk_iterate(w, w->curdir_fd, NULL, arg);
	}

	/* should never occur */
	assert(0);
	return 1;
}

static void walk_strlist(struct walker *w)
{
	unsigned int i;

	if (w->strlist.len == 0) {
		if (w->flags & WF_NO_FILES_STDIN)
			walk_entry(w, NULL);
		return;
	}

	for (i = 0; i < w->strlist.len; i++) {
		int rc = walk_entry(w, w->strlist.list[i].s);
		if (rc)
			return;
	}
}

int walk(struct walker *w, int argc, char **argv)
{
	error_t rc_argp;
	int rc;

	if (w->init) {
		rc = w->init(w, argc, argv);
		if (rc)
			return rc;
	} else
		pu_init();

	rc_argp = argp_parse(w->argp, argc, argv, 0, NULL, NULL);
	if (rc_argp) {
		fprintf(stderr, _("argp_parse: %s\n"), strerror(rc_argp));
		return 1;
	}

	if (w->flags & WF_RECURSE) {
		w->curdir_fd = open(".", O_DIRECTORY);
		if (w->curdir_fd < 0) {
			perror(".");
			return 1;
		}
	} else {
		w->curdir_fd = -1;
	}

	if (w->pre_walk) {
		rc = w->pre_walk(w);
		if (rc)
			return rc;
	}

	walk_strlist(w);

	if (w->post_walk)
		return w->post_walk(w);

	return w->exit_status;
}
