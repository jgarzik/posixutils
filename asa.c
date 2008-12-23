
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

#define _GNU_SOURCE		/* for fgets_unlocked(3) */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libpu.h>


static const char doc[] =
N_("asa - interpret carriage-control characters");

static error_t args_parse_opt (int key, char *arg, struct argp_state *state);

static const struct argp argp = { no_options, args_parse_opt, file_args_doc, doc };

static int asa_actor(struct walker *w, const char *pr_fn, FILE *f)
{
	char *line, buf[LINE_MAX + 1];

	while ((line = fgets_unlocked(buf, sizeof(buf), f)) != NULL) {
		size_t len;
		char ch = *line;

		/* line must be greater than 1 byte long, including newline */
		if (ch == 0) {
			fprintf(stderr, _("malformed line\n"));
			continue;
		}

		/* skip first byte */
		line++;
		len = strlen(line);

		/* trim trailing newline */
		if (line[len - 1] == '\n') {
			len--;
			line[len] = 0;
		}

		/* output based on control character */
		switch (ch) {
		case ' ':
			fputs(line, stdout);
			break;
		case '0':
			putchar('\n');
			fputs(line, stdout);
			break;
		case '1':
			putchar('\f');
			fputs(line, stdout);
			break;
		case '+':
			putchar('\r');
			fputs(line, stdout);
			break;
		}
	}

	return 0;
}

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.cmdline_file		= asa_actor,
};

DECLARE_PU_PARSE_ARGS

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

