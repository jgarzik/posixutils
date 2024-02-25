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
#define _GNU_SOURCE		/* for setlinebuf(3), fgets_unlocked(3) */
#endif
#ifdef __APPLE__
#define _DARWIN_C_SOURCE 1
#endif

#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("comm - compare two sorted files line by line");

static const char args_doc[] = N_("file1 file2");

static struct argp_option options[] = {
	{ NULL, '1', NULL, 0,
	  N_("Suppress the output column of lines unique to file1.") },
	{ NULL, '2', NULL, 0,
	  N_("Suppress the output column of lines unique to file2.") },
	{ NULL, '3', NULL, 0,
	  N_("Suppress the output column of lines duplicated in file1 and file2.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

enum parse_options_bits {
	OPT_FILE1			= (1 << 0),
	OPT_FILE2			= (1 << 1),
	OPT_DUP				= (1 << 2),
};

static int outmask;
static const char *file1, *file2, *lead_dup, *lead_f2;
static char buf1[LINE_MAX + 1], buf2[LINE_MAX + 1];

static void line_out(int ltype, const char *line)
{
	int rc;

	if (ltype & outmask)
		return;

	switch (ltype) {
	case OPT_FILE1:
		rc = fputs(line, stdout);
		break;
	case OPT_FILE2:
		rc = fprintf(stdout, "%s%s", lead_f2, line);
		break;
	case OPT_DUP: {
		rc = fprintf(stdout, "%s%s", lead_dup, line);
		break;
	}
	default:
		rc = 0;
		break;
	}

	if (rc < 0)
		perror("stdout");
}

static int compare_files(void)
{
	FILE *f1, *f2;
	int want1, want2, rc;
	char *l1, *l2;

	rc = ro_file_open(&f1, file1);
	if (rc)
		return 1;

	rc = ro_file_open(&f2, file2);
	if (rc)
		goto out;

	setlinebuf(f1);
	setlinebuf(f2);

	l1 = l2 = NULL;
	want1 = want2 = 1;
	while (want1 || want2) {
		if (want1 && (!l1)) {
			l1 = fgets_unlocked(buf1, sizeof(buf1), f1);
			if (!l1) {
				if (ferror(f1)) {
					perror(file1);
					rc = 1;
					goto out2;
				}
				want1 = 0;
			}
		}
		if (want2 && (!l2)) {
			l2 = fgets_unlocked(buf2, sizeof(buf2), f2);
			if (!l2) {
				if (ferror(f2)) {
					perror(file2);
					rc = 1;
					goto out2;
				}
				want2 = 0;
			}
		}

		if (!l1 && !l2)
			break;

		if (!l1) {
			line_out(OPT_FILE2, l2);
			l2 = NULL;
		} else if (!l2) {
			line_out(OPT_FILE1, l1);
			l1 = NULL;
		} else {
			int cmp = strcoll(l1, l2);
			if (cmp < 0) {		/* l1 < l2 */
				line_out(OPT_FILE1, l1);
				l1 = NULL;
			}
			else if (cmp > 0) {	/* l1 > l2 */
				line_out(OPT_FILE2, l2);
				l2 = NULL;
			}
			else {			/* l1 == l2 */
				line_out(OPT_DUP, l1);
				l1 = l2 = NULL;
			}
		}
	}

out2:
	if (fclose(f2))
		perror(file2);
out:
	if (fclose(f1))
		perror(file1);
	return rc;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case '1':
		outmask |= OPT_FILE1;
		break;
	case '2':
		outmask |= OPT_FILE2;
		break;
	case '3':
		outmask |= OPT_DUP;
		break;

	case ARGP_KEY_ARG:
		switch(state->arg_num) {
		case 0:		file1 = arg; break;
		case 1:		file2 = arg; break;
		default:	argp_usage (state); break; /* too many args */
		}
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 2)		/* not enough args */
			argp_usage (state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	lead_f2 = (outmask & OPT_FILE1) ? "" : "\t";
	if ((outmask & (OPT_FILE1 | OPT_FILE2)) == 0)
		lead_dup = "\t\t";
	else if ((outmask & (OPT_FILE1 | OPT_FILE2)) == (OPT_FILE1 | OPT_FILE2))
		lead_dup = "";
	else
		lead_dup = "\t";

	return compare_files();
}

