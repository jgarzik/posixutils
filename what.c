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

#define _GNU_SOURCE		/* for setlinebuf(3), fgets_unlocked(3) */

#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libpu.h>
#include "cksum_crctab.h"

static const char doc[] =
N_("what - identify SCCS files");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ NULL, 's', NULL, 0,
	  N_("Quit after finding the first occurrence of the pattern in each file") },
	{ }
};

static int what_actor(struct walker *w, const char *fn, FILE *f);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.cmdline_file		= what_actor,
};

static bool opt_short;
static char what_buf[LINE_MAX + 1];


static inline int is_terminator(int ch)
{
	int rc = 0;

	switch(ch) {
	case '"':
	case '>':
	case '\n':
	case '\\':
	case 0:
		rc = 1;
		break;
	default:
		/* do nothing */
		break;
	}

	return rc;
}

static int what_actor(struct walker *w, const char *fn, FILE *f)
{
	char *line;

	setlinebuf(f);

	while ((line = fgets_unlocked(what_buf, sizeof(what_buf), f)) != NULL) {
		char *s, *tmp;
		size_t eatlen, linelen = strlen(line);

restart:
		/* find delimiter */
		s = strstr(line, "@(#)");
		if (!s)
			continue;

		/* advance pointer to just past delimiter */
		s += 4;
		tmp = s;
		eatlen = s - line;

		/* search for terminator */
		while (!is_terminator(*tmp))
			tmp++;
		*tmp = 0;

		/* print string found */
		fprintf(stdout, "%s:\n\t%s\n", fn, s);

		/* check print-only-first-match command line option */
		if (opt_short)
			goto out;

		/* if we did not consume the entire line, inner-loop again */
		eatlen += strlen(s) + 1;
		if (eatlen < linelen) {
			line += eatlen;
			linelen -= eatlen;
			goto restart;
		}
	}

out:
	if (ferror(f)) {
		perror(fn);
		w->exit_status = 1;
	}

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('s', short)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

