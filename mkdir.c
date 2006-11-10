
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

static struct walker walker = {
	.argp			= &argp,
	.cmdline_arg		= mkdir_arg,
};

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
	struct pathelem *pe;

	if ((!strcmp(fn, ".")) || (!strcmp(fn, "/")))
		return 0;

	pe = path_split(fn);

	if (stat(pe->dirn, &st) < 0) {
		if ((!opt_recurse) || (errno != ENOENT)) {
			perror(pe->dirn);
			w->exit_status = EXIT_FAILURE;
			return 0;
		}

		if (opt_recurse)
			mkdir_arg(w, pe->dirn, NULL);
	}
	if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr, _("parent '%s' is not a directory\n"),
			pe->dirn);
		w->exit_status = EXIT_FAILURE;
		return 0;
	}

	path_free(pe);

	/* TODO: handle -m mode argument, using chmod(1)-like mode parser */
	if (mkdir(fn, 0777) < 0) {
		perror(fn);
		w->exit_status = EXIT_FAILURE;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

