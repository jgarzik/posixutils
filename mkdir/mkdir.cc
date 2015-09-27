
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
 *TODO:
 *1) should handle -m mode argument, using chmod(1) mode parser
 *
 *2) change to race-free algorithm:  instead of current recursion, once we
 *have found a parent that exists, fchdir to it, and create each directory
 *component using mkdir(basename)+fchdir(2).
 */


#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("mkdir - make directories");

static struct argp_option options[] = {
	{ "parents", 'p', NULL, 0,
	  N_("Create any missing intermediate pathname components") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static int mkdir_arg(struct walker *w, const char *fn, const struct stat *lst);

static const struct argp argp = { options, parse_opt, file_args_doc, doc };

static struct walker walker;

static bool opt_recurse;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('p', recurse)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int mkdir_arg(struct walker *w, const char *fn, const struct stat *lst)
{
	struct stat st;

	if ((!strcmp(fn, ".")) || (!strcmp(fn, "/")))
		return 0;

	pathelem pe;
	path_split(fn, pe);

	if (stat(pe.dirn.c_str(), &st) < 0) {
		if ((!opt_recurse) || (errno != ENOENT)) {
			perror(pe.dirn.c_str());
			w->exit_status = EXIT_FAILURE;
			return 0;
		}

		if (opt_recurse)
			mkdir_arg(w, pe.dirn.c_str(), NULL);
	}
	if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr, _("parent '%s' is not a directory\n"),
			pe.dirn.c_str());
		w->exit_status = EXIT_FAILURE;
		return 0;
	}

	/* TODO: handle -m mode argument, using chmod(1)-like mode parser */
	if (mkdir(fn, 0777) < 0) {
		perror(fn);
		w->exit_status = EXIT_FAILURE;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.cmdline_arg		= mkdir_arg;
	return walk(&walker, argc, argv);
}

