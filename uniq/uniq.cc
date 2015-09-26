
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE		/* for fgets_unlocked(3) */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libpu.h>


static const char doc[] =
N_("uniq - report or filter out repeated lines in a file");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "count", 'c', NULL, 0,
	  N_("Precede each output line with a count of the number of times the line occurred in the input.") },
	{ }
};

static bool opt_count;
static unsigned int opt_skip;

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };
static int uniq_pre_walk(struct walker *w);
static int uniq_post_walk(struct walker *w);

static int uniq_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	return 0;
}

static struct walker walker;

static char last_line[LINE_MAX + 1];
static unsigned long last_line_dups;
static size_t last_line_len;
static bool have_last_line;

static FILE *outf;

static int write_last_line(void)
{
	int rc;

	if (opt_count)
		rc = fprintf(outf, "%lu %s", last_line_dups, last_line);
	else
		rc = fprintf(outf, "%s", last_line);
	
	return (rc < 0);
}

static int process_line(const char *line)
{
	size_t line_len = strlen(line);
	bool match;

	if (opt_skip > line_len) {
		line = "";
		line_len = 0;
	} else {
		line += opt_skip;
		line_len -= opt_skip;
	}

	match = ((line_len == last_line_len) &&
		 !strcmp(line, last_line));

	if (match) {
		last_line_dups++;
		return 0;
	}

	if (have_last_line && write_last_line())
		return 1;
	
	memcpy(last_line, line, line_len + 1);
	last_line_len = line_len;
	last_line_dups = 1;
	have_last_line = true;

	return 0;
}

static int do_uniq(const char *src_fn, const char *dest_fn)
{
	FILE *f = NULL;
	char line[LINE_MAX + 1];
	bool err = false;

	if (ro_file_open(&f, src_fn))
		return 1;

	if (!dest_fn)
		outf = stdout;
	else {
		outf = fopen(dest_fn, "w");
		if (!outf) {
			perror(dest_fn);
			return 1;
		}
	}

	while (fgets_unlocked(line, sizeof(line), f)) {
		if (process_line(line)) {
			err = true;
			break;
		}
	}

	if (have_last_line && write_last_line())
		err = true;

	if (ferror(f))
		err = true;

	if (outf != stdout && fclose(outf) == EOF)
		perror(dest_fn);
	fclose(f);

	if (err)
		return 1;

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('c', count)
	PU_OPT_ARG

	case ARGP_KEY_END:
		if (state->arg_num < 1)		/* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	PU_OPT_DEFAULT
	PU_OPT_END
}

static int uniq_pre_walk(struct walker *w)
{
	if (w->arglist.size() < 1) {
		fprintf(stderr, _("uniq: missing source file\n"));
		return 1;
	}

	return 0;
}

static int uniq_post_walk(struct walker *w)
{
	return do_uniq(w->arglist[0].c_str(),
		       w->arglist.size() == 1 ? NULL :
		         w->arglist[1].c_str());
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.pre_walk		= uniq_pre_walk;
	walker.post_walk		= uniq_post_walk;
	walker.cmdline_arg		= uniq_actor;
	return walk(&walker, argc, argv);
}
