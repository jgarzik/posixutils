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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <libpu.h>
#include "cksum_crctab.h"

static const char doc[] =
N_("cksum - checksum and count the bytes in a file");

#define options no_options

static error_t args_parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, args_parse_opt, file_args_doc, doc };

enum random_constants {
	CKSUM_BUF_SZ	= 8192
};

static char buf[CKSUM_BUF_SZ];

/* almost directly as presented in POSIX documentation */
static unsigned long updcrc(unsigned long crc_in, const unsigned char *buf,
			    size_t n)
{
	unsigned long s = crc_in;
	unsigned int i, c;

	for (i = n; i > 0; --i) {
		c = (unsigned int) (*buf++);
		s = (s << 8) ^ cksum_crctab[(s >> 24) ^ c];
	}

	return s;
}

static unsigned long fincrc(unsigned long crc_in, size_t n)
{
	unsigned long s = crc_in;
	unsigned int c;

	while (n != 0) {
		c = n & 0377;
		n >>= 8;
		s = (s << 8) ^ cksum_crctab[(s >> 24) ^ c];
	}

	return ~s;
}

static int cksum_fd(struct walker *w, const char *fn, int fd)
{
	ssize_t rc;
	size_t bytes = 0;
	unsigned long crc = 0;
	int have_stdin;

	while (1) {
		rc = read(fd, buf, sizeof(buf));
		if (rc == 0)
			break;
		if (rc < 0) {
			perror(fn);
			return 1;
		}

		crc = updcrc(crc, buf, rc);
		bytes += rc;
	}

	crc = fincrc(crc, bytes);

	have_stdin = (fd == STDIN_FILENO);
	printf("%lu %lu%s%s\n",
	       crc,
	       (unsigned long) bytes,
	       have_stdin ? "" : " ",
	       have_stdin ? "" : fn);

	return 0;
}

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.cmdline_fd		= cksum_fd,
};
DECLARE_PU_PARSE_ARGS

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

