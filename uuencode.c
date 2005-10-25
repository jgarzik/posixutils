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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libpu.h>

#define PFX "uuencode: "

static const char doc[] =
N_("uuencode - encode a binary file");

static const char args_doc[] = N_("[file] decode_pathname");

static struct argp_option options[] = {
	{ "base64", 'm', NULL, 0,
	  N_("Encode the output using the MIME Base64 algorithm") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

enum random_constants {
	UUE_BUF_SZ		= 4096,
	CHUNK_SZ		= 3,
	S_IXALL			= S_IRWXU | S_IRWXG | S_IRWXO,

	LN_MAX			= 45
};

static unsigned char buf[UUE_BUF_SZ];
static unsigned char outbuf[UUE_BUF_SZ];
static int outbuf_len;
static unsigned char *line_start;

static int line_len;
static int line_bytes;

static unsigned char spill[4];
static int n_spill;

static int opt_base64;
static const unsigned char *tbl;

static const unsigned char tbl_base64[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};

static const unsigned char tbl_trad[64] = {
	'`',
	'!', '"', '#', '$', '%', '&', '\'', '(', ')', '*',
	'+', ',', '-', '.', '/', '0', '1', '2', '3', '4',
	'5', '6', '7', '8', '9', ':', ';', '<', '=', '>',
	'?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
	'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',
	']', '^', '_'
};

static void flush_outbuf(void)
{
	if (outbuf_len > 0) {
		write_buf(outbuf, outbuf_len);
		outbuf_len = 0;
	}
}

static void *push_outbuf(const unsigned char *s, size_t len)
{
	void *mem;
	if ((outbuf_len + len) > UUE_BUF_SZ)
		flush_outbuf();

	mem = &outbuf[outbuf_len];

	memcpy(mem, s, len);
	outbuf_len += len;

	return mem;
}

static void push_line (void)
{
	if (!opt_base64)
		*line_start = tbl_trad[line_bytes];

	push_outbuf("\n", 1);

	line_len = 0;
	line_bytes = 0;
	if (!opt_base64)
		line_start = push_outbuf(" ", 1);
}

static void encode_chunk(const unsigned char *s)
{
	unsigned char c[4];
	int ci[4], i;

	/* divide 3 bytes into four 6-bit chunks, reading
	 * from left (MSB) to right
	 */
	ci[0] = ((s[0] >> 2) & 0x3f);

	ci[1] = ((s[0] << 4) & 0x3f);
	ci[1] |= ((s[1] >> 4) & 0xf);

	ci[2] = ((s[1] << 2) & 0x3f);
	ci[2] |= ((s[2] >> 6) & 0x3);

	ci[3] = (s[2] & 0x3f);

	for (i = 0; i < 4; i++)
		c[i] = tbl[ci[i]];

	push_outbuf(c, sizeof(c));
	line_len += sizeof(c);
	line_bytes += CHUNK_SZ;

	if (line_bytes >= LN_MAX)
		push_line();
}

static void finalize_base64(void)
{
	unsigned char c[4];

	c[0] = tbl_base64[ spill[0] >> 2 ];

	if (n_spill == 1) {
		c[1] = tbl_base64[ ((spill[0] & 0x3) << 4) ];
		c[2] = '=';
	} else {
		c[1] = tbl_base64[ ((spill[0] & 0x3) << 4) | (spill[1] >> 4) ];
		c[2] = tbl_base64[ ((spill[1] & 0xf) << 2) ];
	}

	c[3] = '=';

	push_outbuf(c, sizeof(c));
	line_len += sizeof(c);
	line_bytes += n_spill;
}

static void finalize_traditional(void)
{
	memset(&spill[n_spill], 0, CHUNK_SZ - n_spill);
	encode_chunk(spill);
	n_spill = 0;
}

static void flush_last_line (void)
{
	if (n_spill) {
		if (opt_base64)
			finalize_base64();
		else
			finalize_traditional();
	}

	push_line();

	if (!opt_base64) {
		*line_start = '`';
		push_outbuf("\n", 1);
	}
}

static void push_bytes(ssize_t len)
{
	const unsigned char *s = buf;

	if (n_spill) {
		while ((n_spill < CHUNK_SZ) && (len > 0)) {
			spill[n_spill] = *s;
			s++;
			len--;
			n_spill++;
		}

		if (n_spill < CHUNK_SZ)
			return;

		encode_chunk(spill);
		n_spill = 0;
	}

	while (len >= CHUNK_SZ) {
		encode_chunk(s);
		s += CHUNK_SZ;
		len -= CHUNK_SZ;
	}

	if (len > 0) {
		memcpy(spill, s, len);
		n_spill = len;
	}
}

static void do_header(int fd, const char *fn_string)
{
	struct stat st;
	char linebuf[PATH_MAX + 20];

	if (fstat(fd, &st) == 0)
		st.st_mode &= S_IXALL;
	else
		st.st_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	snprintf(linebuf, sizeof(linebuf), "begin%s %03o %s\n",
		 opt_base64 ? "-base64" : "", st.st_mode, fn_string);
	push_outbuf(linebuf, strlen(linebuf));
}

static int do_encode(int fd, const char *fn_string)
{
	ssize_t rc;

	if (opt_base64)
		tbl = tbl_base64;
	else
		tbl = tbl_trad;

	do_header(fd, fn_string);

	if (!opt_base64)
		line_start = push_outbuf(" ", 1);
	line_len = 0;
	line_bytes = 0;

	while (1) {
		rc = read(fd, buf, sizeof(buf));
		if (rc == 0)
			break;
		if (rc < 0) {
			perror(fn_string);
			return 1;
		}

		push_bytes(rc);
	}

	flush_last_line();

	if (opt_base64)
		push_outbuf("====\n", 5);
	else
		push_outbuf("end\n", 6);

	flush_outbuf();

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'm':
		opt_base64 = 1;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	char *input_fn = NULL, *fn_name;
	int fd, rc;
	int idx;
	error_t arc;

	pu_init();

	arc = argp_parse(&argp, argc, argv, 0, &idx, NULL);
	if (idx < 0)
		return 1;

	if ((argc - idx) == 1)
		fn_name = argv[idx];
	else if ((argc - idx) == 2) {
		input_fn = argv[idx++];
		fn_name = argv[idx];
	}
	else {
		fprintf(stderr, PFX "invalid arguments\n");
		return 1;
	}

	if (input_fn) {
		fd = open(input_fn, O_RDONLY);
		if (fd < 0) {
			perror(input_fn);
			return 1;
		}

		/* Do we care about the return value of posix_fadvise(2) ? */
		posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
	} else {
		fd = STDIN_FILENO;
	}

	rc = do_encode(fd, fn_name);

	if (input_fn) {
		if (close(fd) < 0)
			rc |= 1;
	}

	return rc;
}

