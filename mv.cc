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
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>
#include <libpu.h>


#define PFX "mv: "

static const char doc[] =
N_("mv - move (rename) files");

static struct argp_option options[] = {
	{ "force", 'f', NULL, 0,
	  N_("Do not prompt for confirmation if the destination path exists.") },
	{ "interactive", 'i', NULL, 0,
	  N_("Prompt for confirmation if the destination path exists") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, file_args_doc, doc };


static int opt_force;
static int opt_interactive;
static const char *target_dir;
static int target_fd = -1, curdir_fd = -1;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'i':
		opt_interactive = 1;
		break;
	case 'f':
		opt_force = 1;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static void open_target_curdir(void)
{
	if (target_dir && target_fd < 0) {
		target_fd = open(target_dir, O_DIRECTORY);
		if (target_fd < 0) {
			perror(target_dir);
			exit(1);
		}
	}

	if (curdir_fd < 0) {
		curdir_fd = open(".", O_DIRECTORY);
		if (curdir_fd < 0) {
			perror(".");
			exit(1);
		}
	}
}

static int should_ask(const char *fn)
{
	if (!isatty(STDIN_FILENO))
		return 0;
	if (access(fn, W_OK) == 0)
		return 0;
	return 1;
}

static int copy_special(const char *src, const struct stat *st,
			const char *target)
{
	/* TODO */
	return -1;
}

static int copy_attributes(const char *fn, int dest_fd,
			   const struct stat *st, int have_err)
{
	mode_t mode, tmp_mode;

	if (fchown(dest_fd, st->st_uid, st->st_gid) < 0) {
		perror(fn);
		have_err = 1;
	}

	mode = st->st_mode;
	tmp_mode = mode & ~(S_ISUID | S_ISGID);

	/* quoth SUS, "If the user ID, group ID, or file mode of a
	 * regular file cannot be duplicated, the file mode bits S_ISUID
	 * and S_ISGID shall not be duplicated."
	 */

	if (fchmod(dest_fd, tmp_mode) < 0) {
		perror(fn);
		return 1;
	}

	/* if no error, and setuid bits present, we set those bits
	 * in a separate step
	 */
	if ((!have_err) && (mode != tmp_mode)) {
		if (fchmod(dest_fd, mode) < 0) {
			perror(fn);
			return 1;
		}
	}

	return have_err;
}

static int copy_recurse(const char *src, const struct stat *st)
{
	return -1;
}

static int copy_reg_file(const char *src, struct stat *st, const char *target)
{
	struct utimbuf ut;
	int attr_rc = 0, rc = 0, in_fd = -1, out_fd = -1;
	ssize_t crc;

	/* open input file */
	in_fd = open(src, O_RDONLY);
	if (in_fd < 0) {
		perror(src);
		return 1;
	}

	/* Do we care about the return value of posix_fadvise(2) ? */
	posix_fadvise(in_fd, 0, 0, POSIX_FADV_SEQUENTIAL);

	/* st variable may be stale by now */
	if (fstat(in_fd, st) < 0) {
		perror(src);
		rc = 1;
		goto out;
	}

	/* open output file */
	out_fd = open(target, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (out_fd < 0) {
		perror(target);
		rc = 1;
		goto out;
	}

	/* copy entire input to output */
	crc = copy_fd(target, out_fd, src, in_fd);
	if (crc < 0)
		rc = 1;

	/* sigh... we need fd-based futime(2) */
	ut.actime = st->st_atime;
	ut.modtime = st->st_mtime;
	if (utime(target, &ut) < 0) {
		perror(target);
		attr_rc = rc = 1;
	}

	rc |= copy_attributes(target, out_fd, st, attr_rc);

out:
	if (in_fd >= 0)
		if (close(in_fd) < 0)
			perror(src);
	if (out_fd >= 0)
		if (close(out_fd) < 0)
			perror(target);
	return rc;
}

static int copy_file(const char *src, const char *dest_dir,
		     const char *dest_base, const char *dest, int recurse)
{
	struct stat st;

	/* check and see if we're "copying" something other than
	 * a regular file
	 */
	if (lstat(src, &st) < 0) {
		perror(src);
		return 1;
	}

	if (S_ISDIR(st.st_mode) && (!recurse)) {
		fprintf(stderr, "mv: attempting to copy directory '%s' as file\n",
			src);
		return 1;
	}

	/* attempt to remove target, if it exists */
	if (access(dest, F_OK) == 0) {
		if (unlink(dest) < 0) {
			perror(dest);
			return 1;
		}
	}

	if (S_ISDIR(st.st_mode))
		return copy_recurse(src, &st);

	if (!S_ISREG(st.st_mode))
		return copy_special(src, &st, dest);

	return copy_reg_file(src, &st, dest);
}

static int move_file(const char *fn, const char *target)
{
	int rc;

	if ((!opt_force) &&
	    (access(target, F_OK) == 0) &&
	    ((opt_interactive) || should_ask(target))) {
		rc = ask_question(PFX, "%soverwrite '%s'? ", fn);
		if (!rc)
			return 1;
	}

	rc = rename(fn, target);
	if (rc == 0)
		return 0;
	if (errno != EXDEV) {
		fprintf(stderr, _("rename '%s' to '%s': %s\n"),
			fn, target, strerror(errno));
		return 1;
	}

	return 2;	/* on another fs; needs copying */
}

static int mv_fn_actor(struct cmdline_walker *cw, const char *fn)
{
	struct pathelem *pe;
	char *new_fn;
	int rc;

	pe = path_split(fn);
	new_fn = strpathcat(target_dir, pe->basen);

	rc = move_file(fn, new_fn);
	if (rc != 2)
		goto out;

	open_target_curdir();

	rc = copy_file(fn, target_dir, pe->basen, new_fn, 1);

out:
	path_free(pe);
	free(new_fn);
	return rc;
}

static int do_2arg_form(int argc, char **argv, int idx)
{
	struct pathelem *pe;
	char *fn;
	int rc;

	if ((argc - idx) != 2) {
		fprintf(stderr, _("mv: too many arguments, when "
				"target is not directory\n"));
		return 1;
	}

	fn = argv[idx];

	rc = move_file(fn, target_dir);
	if (rc != 2)	/* moved successfully, or error */
		return rc;

	pe = path_split(target_dir);

	rc = copy_file(fn, pe->dirn, pe->basen, target_dir, 0);

	path_free(pe);

	return rc;
}

int main (int argc, char *argv[])
{
	struct cmdline_walker walker;
	walker.argc		= argc;
	walker.argv		= argv;
	walker.argp		= &argp;
	walker.flags		= CFL_SKIP_LAST;
	walker.fn_actor		= mv_fn_actor;

	int rc, idx, have_2arg_form = 1;
	struct stat st;

	pu_init();

	idx = parse_cmdline(&walker);
	if (idx < 0)
		return 1;

	if ((argc - idx) < 2) {
		fprintf(stderr, _("missing source/target\n"));
		return 1;
	}

	target_dir = argv[argc - 1];
	rc = stat(target_dir, &st);
	if ((rc == 0) && (S_ISDIR(st.st_mode)))
		have_2arg_form = 0;

	if (have_2arg_form)
		return do_2arg_form(argc, argv, idx);

	return __walk_cmdline(&walker, idx);
}

