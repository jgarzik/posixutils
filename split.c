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

#define _GNU_SOURCE		/* for O_DIRECTORY, fgets_unlocked(3) */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <libpu.h>
#include <assert.h>


static const char doc[] =
N_("split - split a file into pieces");

static const char args_doc[] = N_("[file [prefix]]");

static struct argp_option options[] = {
	{ "suffix-length", 'a', "N", 0,
	  N_("Use N letters to form the suffix portion of the filenames of the split file") },
	{ "bytes", 'b', "N[km]", 0,
	  N_("Split a file into pieces n bytes in size") },
	{ "lines", 'l', "N", 0,
	  N_("Specify the number of lines in each resulting file piece") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

enum piece_mode_t {
	MODE_NONE,
	MODE_BYTE,
	MODE_LINE,
};


static char			inbuf[8192];
static const char		*err_fn;
static char			*suffix;

static uintmax_t		output_count;
static int			output_fd = -1;
static char			*output_fn;

static int			opt_suffix_len	= 2;
static uintmax_t		opt_piece_size	= 1000;
static enum piece_mode_t	opt_piece_mode	= MODE_LINE;
static const char		*opt_prefix	= "x";
static const char		*opt_input_fn;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'a': {
		int rc, tmp;

		rc = sscanf(arg, "%d", &tmp);
		if ((rc != 1) || (tmp < 1) || (tmp >= NAME_MAX))
			return ARGP_ERR_UNKNOWN;

		opt_suffix_len = tmp;
		break;
	}

	case 'b': {
		int rc;
		char ch;
		uintmax_t tmp, mult;

		rc = sscanf(arg, "%" PRIuMAX "%c", &tmp, &ch);
		if ((rc < 1) || (rc > 2) || (tmp < 1))
			return ARGP_ERR_UNKNOWN;
		if ((rc > 1) && (ch != 'k') && (ch != 'm'))
			return ARGP_ERR_UNKNOWN;
		if (rc == 1)
			ch = 0;
		else
			ch = tolower(ch);

		switch (tolower(ch)) {
		case 'k': mult = 1024; break;
		case 'm': mult = 1024 * 1024; break;
		default: mult = 1; break;
		}

		opt_piece_mode = MODE_BYTE;
		opt_piece_size = tmp * mult;
		break;
	}

	case 'l': {
		int rc;
		uintmax_t tmp;

		rc = sscanf(arg, "%" PRIuMAX, &tmp);
		if ((rc != 1) || (tmp < 1))
			return ARGP_ERR_UNKNOWN;

		opt_piece_mode = MODE_LINE;
		opt_piece_size = tmp;
		break;
	}

	case ARGP_KEY_ARG:
		switch(state->arg_num) {
		case 0:		opt_input_fn = arg; break;
		case 1:		opt_prefix = arg; break;
		default:	argp_usage (state); break; /* too many args */
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int incr_suffix(void)
{
	int i;

	if (!suffix) {
		suffix = xmalloc(opt_suffix_len + 1);
		for (i = 0; i < opt_suffix_len; i++)
			suffix[i] = 'a';
		suffix[opt_suffix_len] = 0;
		return 0;
	}

	for (i = opt_suffix_len - 1; i >= 0; i--) {
		if (suffix[i] != 'z') {
			suffix[i]++;
			return 0;
		}

		suffix[i] = 'a';
	}

	return 1;
}

static int open_output(void)
{
	int rc;

	if (output_fd < 0) {
		rc = incr_suffix();
		if (rc)
			return rc;

		assert(output_fn == NULL);
		output_fn = xmalloc(opt_suffix_len + strlen(opt_prefix) + 1);
		sprintf(output_fn, "%s%s", opt_prefix, suffix);

		output_fd = open(output_fn, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (output_fd < 0) {
			perror(output_fn);
			return 1;
		}

		/* Do we care about the return value of posix_fadvise(2) ? */
		posix_fadvise(output_fd, 0, 0, POSIX_FADV_SEQUENTIAL);
	}

	return 0;
}

static int close_output(void)
{
	int rc = 0;

	if (output_fd < 0)
		return 0;

	if (close(output_fd) < 0) {
		perror(output_fn);
		rc = 1;
	}
	free(output_fn);

	output_fd = -1;
	output_fn = NULL;
	output_count = 0;

	return rc;
}

static int incr_output(uintmax_t incr)
{
	int rc = 0;

	output_count += incr;
	assert(output_count <= opt_piece_size);

	if (output_count == opt_piece_size)
		rc = close_output();

	return rc;
}

static int output_bytes(const char *buf, size_t len)
{
	int rc;
	uintmax_t dist;
	size_t wlen;

	while (len > 0) {
		rc = open_output();
		if (rc)
			return rc;

		dist = opt_piece_size - output_count;
		if (dist < len)
			wlen = dist;
		else
			wlen = len;

		rc = write_fd(output_fd, buf, wlen, output_fn);
		if (rc)
			return rc;

		buf += wlen;
		assert(wlen <= len);
		len -= wlen;

		rc = incr_output(wlen);
		if (rc)
			return rc;
	}

	return 0;
}

static int output_line(const char *s)
{
	int rc = open_output();
	if (rc)
		return rc;

	rc = write_fd(output_fd, s, strlen(s), output_fn);
	if (rc)
		return rc;

	return incr_output(1);
}

static int split_bytes(FILE *f)
{
	int done = 0;

	if (setvbuf(f, NULL, _IONBF, 0)) {
		perror(_("setvbuf(3) failed"));
		return 1;
	}

	while (!done) {
		size_t n;
		int rc;

		n = fread(inbuf, 1, sizeof(inbuf), f);
		if (ferror(f))
			return 1;
		if (feof(f))
			done = 1;

		rc = output_bytes(inbuf, n);
		if (rc)
			return rc;
	}

	return 0;
}

static int split_lines(FILE *f)
{
	char *s;

	setlinebuf(f);

	while ((s = fgets_unlocked(inbuf, sizeof(inbuf), f)) != NULL) {
		int rc = output_line(s);
		if (rc)
			return rc;
	}

	return 0;
}

static int execute_split(void)
{
	FILE *f;
	int rc = 0;

	if ((opt_input_fn == NULL) || (!strcmp(opt_input_fn, "-"))) {
		f = stdin;
		err_fn = _("(standard input)");
	} else {
		err_fn = opt_input_fn;
		if (ro_file_open(&f, opt_input_fn))
			return 1;
	}

	if (opt_piece_mode == MODE_BYTE)
		rc = split_bytes(f);
	else if (opt_piece_mode == MODE_LINE)
		rc = split_lines(f);
	else {
		assert(0);
		rc = 1;
	}

	if (ferror(f)) {
		perror(err_fn);
		rc = 1;
	}

	rc |= close_output();

	if (opt_input_fn != NULL)
		if (fclose(f))
			perror(opt_input_fn);

	return EXIT_FAILURE;
}

int main (int argc, char *argv[])
{
	struct pathelem *pe;
	error_t rc;

	pu_init();

	rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (rc) {
		fprintf(stderr, _("argp_parse failed: %s\n"), strerror(rc));
		return 1;
	}

	pe = path_split(opt_prefix);
	if ((strlen(pe->basen) + opt_suffix_len) > NAME_MAX) {
		fprintf(stderr, _("prefix + suffix too large\n"));
		return 1;
	}
	path_free(pe);

	return execute_split();
}

