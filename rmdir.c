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

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <libpu.h>

static const char doc[] =
N_("rmdir - remove empty directories");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "parents", 'p', NULL, 0,
	  N_("Remove all directories in a pathname.") },
	{ }
};

static int rmdir_fn_actor(struct walker *w, const char *fn, const struct stat *st);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp				= &argp,
	.cmdline_arg			= rmdir_fn_actor,
};

static char pathbuf[PATH_MAX + 2];
static bool opt_recurse;


static int do_rmdir(const char *fn)
{
	if (rmdir(fn) < 0) {
		perror(fn);
		return 1;
	}

	return 0;
}

static int rmdir_fn_actor(struct walker *w, const char *fn, const struct stat *st)
{
	int rc;
	char *dn;

	rc = do_rmdir(fn);
	if (rc || !opt_recurse)
		return rc;

	strncpy(pathbuf, fn, sizeof(pathbuf));
	pathbuf[sizeof(pathbuf) - 1] = 0;

	dn = pathbuf;
	while (!rc) {
		dn = dirname(dn);
		if (!dn || (*dn == 0) || (!strcmp(dn, "/"))) {
			rc = 0;
			break;
		}
		rc = do_rmdir(dn);
	}

	return rc;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('p', recurse)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

