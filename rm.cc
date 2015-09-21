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
#define _GNU_SOURCE		/* for O_DIRECTORY */
#define _DARWIN_C_SOURCE 1		/* for O_DIRECTORY */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <libpu.h>


#define PFX "rm: "

static const char doc[] =
N_("rm - remove files or directories");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "force", 'f', NULL, 0,
	  N_("Do not prompt for confirmation. Do not write diagnostic messages or modify the exit status in the case of nonexistent operands.") },
	{ "interactive", 'i', NULL, 0,
	  N_("Prompt for confirmation") },
	{ "recursive", 'R', NULL, 0,
	  N_("Remove file hierarchies") },
	{ NULL, 'r', NULL, OPTION_ALIAS },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };


static int opt_force;
static int opt_recurse;
static int opt_interactive;


static int rm(int dirfd, const char *dirn, const char *basen);


static int can_write(const struct stat *st)
{
	uid_t uid;
	gid_t gid;

	if (st->st_mode & S_IWOTH)
		return 1;

	uid = geteuid();
	if ((uid == st->st_uid) && (st->st_mode & S_IWUSR))
		return 1;

	gid = getegid();
	if ((gid == st->st_gid) && (st->st_mode & S_IWGRP))
		return 1;

	return 0;
}

static int should_prompt(const struct stat *st)
{
	if (opt_interactive)
		return 1;

	if (!can_write(st) && isatty(STDIN_FILENO))
		return 1;

	return 0;
}

static int rm(int dirfd, const char *dirn, const char *basen)
{
	struct stat st;
	int rc = 0, isdir;
	char *fn;

	if (have_dots(basen))
		return 0;

	fn = strpathcat(dirn, basen);

	if (lstat(basen, &st) < 0) {
		if (!opt_force)
			perror(fn);
		rc = 1;
		goto out;
	}

	isdir = S_ISDIR(st.st_mode);
	if (isdir) {
		if (!opt_recurse) {
			fprintf(stderr, PFX "ignoring directory '%s'\n", fn);
			rc = 1;
			goto out;
		}

		if ((!opt_force) && should_prompt(&st)) {
			int i = ask_question(PFX, "%srecurse into '%s'?  ", fn);
			if (!i)
				goto out;
		}

		rc = iterate_directory(dirfd, dirn, basen, opt_force, rm);

		if (rmdir(basen) < 0) {
			perror(fn);
			rc = 1;
		}
	} else {
		if ((!opt_force) && should_prompt(&st)) {
			int i = ask_question(PFX, "%sremove '%s'?  ", fn);
			if (!i)
				goto out;
		}

		if (unlink(basen) < 0) {
			perror(fn);
			rc = 1;
		}
	}

out:
	free(fn);
	return rc;
}

static int rm_fn_actor(struct cmdline_walker *cw, const char *fn)
{
	struct pathelem *pe;
	int rc = 0, old_dirfd, dirfd;

	pe = path_split(fn);

	if (have_dots(pe->basen)) {
		rc = 1;
		goto out;
	}

	/* get ref to current dir */
	old_dirfd = open(".", O_DIRECTORY);
	if (old_dirfd < 0) {
		perror(".");
		rc = 1;
		goto out;
	}

	/* get ref to destination dir */
	dirfd = open(pe->dirn, O_DIRECTORY);
	if (dirfd < 0) {
		if (!opt_force)
			perror(pe->dirn);
		rc = 1;
		goto out_old;
	}

	/* change to dest dir */
	if (fchdir(dirfd) < 0) {
		perror(pe->dirn);
		rc = 1;
		goto out_fd;
	}

	/* remove file */
	rc = rm(dirfd, pe->dirn, pe->basen);

	/* change back to original dir */
	if (fchdir(old_dirfd) < 0) {
		perror(".");
		rc = 1;
	}

out_fd:
	if (close(dirfd) < 0)
		perror(pe->dirn);
out_old:
	if (close(old_dirfd) < 0)
		perror(".");
out:
	path_free(pe);
	return rc;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'i':
		opt_interactive = 1;
		break;
	case 'f':
		opt_force = 1;
		break;
	case 'R':
	case 'r':
		opt_recurse = 1;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	struct cmdline_walker walker;
	walker.argc	= argc;
	walker.argv	= argv;
	walker.argp	= &argp;
	walker.flags	= 0;
	walker.fn_actor	= rm_fn_actor;

	pu_init();

	return walk_cmdline(&walker);
}

