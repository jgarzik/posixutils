/*
 * Copyright 2004-2005 Jeff Garzik <jgarzik@pobox.com>
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

#define _GNU_SOURCE		/* for O_DIRECTORY */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libpu.h>


static const char doc[] =
N_("pathchk - check whether file names are valid or portable");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "portability", 'p', NULL, 0,
	  N_("Base checks on universal POSIX limits, not path-specific limits") },
	{ }
};

static int pathchk_fn_actor(struct walker *w, const char *fn, const struct stat *lst);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.cmdline_arg		= pathchk_fn_actor,
};

static bool opt_portable;

static size_t path_max, name_max;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('P', portable)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static char *find_fshandle(const char *path)
{
	struct pathelem *pe;
	struct stat st;
	char *ret_fshandle;

	if ((lstat(path, &st) == 0) ||
	    (!strcmp(path, "/")) ||
	    (!strcmp(path, ".")))
		return xstrdup(path);

	pe = path_split(path);

	ret_fshandle = find_fshandle(pe->dirn);

	path_free(pe);

	return ret_fshandle;
}

static int check_component(const char *basen)
{
	if (strlen(basen) > name_max) {
		fprintf(stderr, _("component %s: length %u exceeds limit %u\n"),
			basen, strlen(basen), name_max);
		return 1;
	}

	if (opt_portable) {
		unsigned int i, len = strlen(basen);

		for (i = 0; i < len; i++) {
			if (!is_portable_char(basen[i])) {
				fprintf(stderr, _("component %s contains non-portable char\n"), basen);
				return 1;
			}
		}
	}

	return 0;
}

static int check_path(const char *path)
{
	struct pathelem *pe;
	int rc;

	pe = path_split(path);

	rc = check_component(pe->basen);

	if ((rc == 0) && strcmp(pe->dirn, "/") && strcmp(pe->dirn, "."))
		rc |= check_path(pe->dirn);

	path_free(pe);

	return rc;
}

static int pathchk_fn_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	if (opt_portable) {
		path_max = _POSIX_PATH_MAX;
		name_max = _POSIX_NAME_MAX;
	} else {
		char *fshandle = find_fshandle(fn);
		long l;

		l = pathconf(fshandle, _PC_PATH_MAX);
		if (l < 0) {
			perror(fshandle);
			free(fshandle);
			return 1;
		}
		path_max = (size_t) l;

		l = pathconf(fshandle, _PC_NAME_MAX);
		if (l < 0) {
			perror(fshandle);
			free(fshandle);
			return 1;
		}
		name_max = (size_t) l;

		free(fshandle);
	}

	if (strlen(fn) > path_max) {
		fprintf(stderr, "%s: path length %u exceeds limit %u\n",
			fn, strlen(fn), path_max);
		return 1;
	}

	return check_path(fn);
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

