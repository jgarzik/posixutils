
/*
 * Copyright 2008-2009 Red Hat, Inc.
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libpu.h>

static const char doc[] =
N_("cmp - compare two files");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "verbose", 'l', NULL, 0,
	  N_("Write the byte number (decimal) and the differing bytes (octal) for each difference.") },
	{ "silent", 's', NULL, 0,
	  N_("Write nothing for differing files; return exit status only.") },
	{ }
};

static int cmp_actor(struct walker *w, const char *fn, const struct stat *lst);
static int cmp_post_walk(struct walker *w);
static int cmp_pre_walk(struct walker *w);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.pre_walk		= cmp_pre_walk,
	.post_walk		= cmp_post_walk,
	.cmdline_arg		= cmp_actor,
};


static bool opt_verbose;
static bool opt_silent;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('l', verbose)
	PU_OPT_SET('s', silent)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int cmp_pre_walk(struct walker *w)
{
	if (w->strlist.len != 2) {
		fprintf(stderr, _("cmp: two pathnames required for comparison\n"));
		return 2;
	}

	return 0;
}

static int cmp_post_walk(struct walker *w)
{
	const char *file1, *file2;
	FILE *f1, *f2;
	int c1, c2;
	int rc = 0;
	bool diff = false;
	unsigned long long lines = 1;
	unsigned long long bytes = 0;

	file1 = slist_ref(&w->strlist, 0);
	file2 = slist_ref(&w->strlist, 1);

	f1 = fopen(file1, "r");
	if (!f1) {
		perror(file1);
		return 2;
	}

	f2 = fopen(file2, "r");
	if (!f2) {
		perror(file2);
		rc = 2;
		goto out;
	}

	while (1) {
		c1 = getc(f1);
		c2 = getc(f2);

		bytes++;

		if (c1 == '\n')
			lines++;

		if (c1 == EOF || c2 == EOF) {
			if (c1 != c2) {
				fprintf(stderr, _("cmp: EOF on %s\n"),
					c1 == EOF ? file1 : file2);
				rc = 1;
			}
			break;
		}

		else if (!diff && c1 == c2)
			continue;

		/* handle difference */
		diff = true;

		/* if silent, just return 1 and stop */
		if (opt_silent) {
			rc = 1;
			break;
		}

		/* if not verbose (long format), print info and stop */
		else if (!opt_verbose) {
			fprintf(stdout,
				_("%s %s differ: char %llu, line %llu\n"),
				file1, file2, bytes, lines);
			rc = 1;
			break;
		}

		/* print differing bytes, and repeat */
		fprintf(stdout, "%llu %o %o\n", bytes, c1, c2);
	}

out:
	if (f1)
		fclose(f1);
	if (f2)
		fclose(f2);

	return rc;
}

static int cmp_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	return 0;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
