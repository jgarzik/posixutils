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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libpu.h>

static const char doc[] =
N_("head - output the first part of files");

static struct argp_option options[] = {
	{ "lines", 'n', "N", 0,
	  N_("first number lines of each input file to copy to stdout") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static int head_fd(struct walker *w, const char *fn, int fd);
static int head_pre_walk(struct walker *w);
static const struct argp argp = { options, parse_opt, file_args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.pre_walk		= head_pre_walk,
	.cmdline_fd		= head_fd,
};

enum random_constants {
	HEAD_BUF_SZ = 8192
};

static char buf[HEAD_BUF_SZ];
static int head_lines = 10;
static int print_header;

static void do_print_header(const char *fn)
{
	if (print_header)
		printf("\n==> %s <==\n", fn);
}

static inline const char *find_nl(const char *s, size_t len)
{
	return memchr(s, '\n', len);
}

static int head_fd(struct walker *w, const char *fn, int fd)
{
	ssize_t rc;
	int err;
	unsigned int line_len, len, lines = 0;
	const char *s, *nl;

	do_print_header(fn);

	while (1) {
		rc = read(fd, buf, sizeof(buf));
		if (rc < 0) {
			perror(fn);
			return 1;
		}
		if (rc == 0)
			break;

		nl = buf;
		len = rc;
		s = find_nl(nl, len);
		while ((s) && (lines < head_lines)) {
			lines++;
			line_len = s - nl + 1;
			nl += line_len;
			len -= line_len;
			s = find_nl(nl, len);
		}

		if (lines == head_lines) {
			err = write_buf(buf, nl - buf);
			return 0;
		}

		err = write_buf(buf, rc);
		if (err)
			return err;
	}

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 'n': {
		int rc, tmp = 0;

		rc = sscanf(arg, "%d", &tmp);
		if ((rc != 1) || (tmp <= 0))
			return ARGP_ERR_UNKNOWN;

		head_lines = tmp;
		break;
	}

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int head_pre_walk(struct walker *w)
{
	if (w->strlist.len > 1)
		print_header = 1;

	return 0;
}

int main(int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
