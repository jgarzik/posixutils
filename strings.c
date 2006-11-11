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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <libpu.h>

static const char doc[] =
N_("strings - find printable strings in files");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "all", 'a', NULL, 0,
	  N_("Scan files in their entirety") },
	{ "bytes", 'n', "NUMBER", 0,
	  N_("Specify the minimum string length") },
	{ "radix", 't', "FORMAT", 0,
	  N_("Write each string preceded by its byte offset from the start of the file") },
	{ }
};

static int strings_actor(struct walker *w, const char *fn, int fd);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.cmdline_fd		= strings_actor,
};

static int opt_strlen = 4;
static enum opt_prefix {
	PFX_NONE,
	PFX_DECIMAL,
	PFX_OCTAL,
	PFX_HEX
} opt_prefix = PFX_NONE;

enum random_constants {
	BUFLEN			= 8192,
};

static struct buf_state {
	char	buf[BUFLEN];
	int	used;
	off_t	offset;
} st;

static char *find_sep(char *s, size_t n)
{
	int ch;

	while (n > 0) {
		ch = *s;
		if ((ch == '\n') || (ch == 0))
			return s;

		s++;
		n--;
	}

	return NULL;
}

static int string_print(const char *s)
{
	char ofs[64];

	switch (opt_prefix) {
	case PFX_NONE:
		ofs[0] = 0;
		break;
	case PFX_DECIMAL:
		sprintf(ofs, "%Lu", (unsigned long long) st.offset);
		break;
	case PFX_OCTAL:
		sprintf(ofs, "%Lo", (unsigned long long) st.offset);
		break;
	case PFX_HEX:
		sprintf(ofs, "%Lx", (unsigned long long) st.offset);
		break;
	}

	return printf("%s%s\n", ofs, s);
}

static int strings_buf(void)
{
	char *p, *sep = NULL, *lastsep;
	size_t len, diff;
	int i;

	p = st.buf;
	len = st.used;
	while (1) {
		lastsep = sep;
		sep = find_sep(p, len);
		if (!sep) {
			if (len > (BUFLEN / 2)) {
				diff = len - (BUFLEN / 2);
				p += diff;
				len -= diff;
			}

			assert(p != st.buf);

			memmove(st.buf, p, len);
			st.used = len;
			st.offset += p - st.buf;

			return 0;
		}

		*sep = 0;

		diff = sep - p;
		for (i = 0; i < diff; i++)
			if (!isprint(p[diff - i - 1]))
				break;

		if (i >= opt_strlen)
			if (string_print(p) < opt_strlen)
				return 1;

		p = sep + 1;
		len -= diff + 1;
	}
}

static int strings_actor(struct walker *w, const char *fn, int fd)
{
	ssize_t rrc;

	memset(&st, 0, sizeof(st));

	while (1) {
		rrc = read(fd, st.buf + st.used, BUFLEN - st.used);
		if (rrc < 0) {
			fprintf(stderr, "\n");
			perror(fn);
			return 1;
		}
		if (rrc == 0)
			break;

		st.used += rrc;

		if (strings_buf())
			return 1;
	}

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	long l;
	char *endp = NULL;

	PU_OPT_BEGIN
	PU_OPT_IGNORE('a')

	case 'n':
		errno = 0;
		l = strtol(arg, &endp, 10);
		if (errno || (l < 1))
			argp_usage(state);
		opt_strlen = l;
		break;
	case 't':
		break;

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
