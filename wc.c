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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <libpu.h>

static const char doc[] =
N_("wc - print the number of newlines, words, and bytes in files");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "bytes", 'c', NULL, 0,
	  N_("Write to the standard output the number of bytes in each input file") },
	{ "lines", 'l', NULL, 0,
	  N_("Write to the standard output the number of <newline>s in each input file") },
	{ "words", 'w', NULL, 0,
	  N_("Write to the standard output the number of words in each input file") },
	{ "chars", 'm', NULL, 0,
	  N_("Write to the standard output the number of characters in each input file") },
	{ }
};

static int wc_post_walk(struct walker *w);
static int wc_actor(struct walker *w, const char *fn, int fd);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.post_walk		= wc_post_walk,
	.cmdline_fd		= wc_actor,
};

enum {
	WC_CHAR		= (1 << 0),
	WC_LINE		= (1 << 1),
	WC_WORD		= (1 << 2),
	WC_ALL		= WC_CHAR | WC_LINE | WC_WORD,

	WCT_BYTE	= 0,
	WCT_CHAR	= 1,

	WC_BUFSZ	= 4096,
};

struct count_info {
	unsigned long	bytes;
	unsigned long	chars;
	unsigned long	words;
	unsigned long	lines;

	bool		in_word;
	bool		in_line;
};


static int outmask;
static int out_type;
static struct count_info totals;
static int total_files;

static char wc_buf[WC_BUFSZ];
static wchar_t wc_buf_mb[WC_BUFSZ];


static void count_buf_mb(struct count_info *info, const wchar_t *buf,
			 size_t len)
{
	while (len > 0) {
		wchar_t c;

		c = *buf;

		if (iswspace(c)) {
			if (info->in_word) {
				info->in_word = false;
				info->words++;
			}
			if (c == L'\n') {
				info->in_line = false;
				info->lines++;
			} else
				info->in_line = true;
		} else {
			info->in_word = true;
		}

		buf++;
		len--;
		info->chars++;
	}
}

static void count_buf(struct count_info *info, const char *buf, size_t len)
{
	while (len > 0) {
		char c;

		c = *buf;

		if (isspace(c)) {
			if (info->in_word) {
				info->in_word = false;
				info->words++;
			}
			if (c == '\n') {
				info->in_line = false;
				info->lines++;
			} else
				info->in_line = true;
		} else {
			info->in_word = true;
		}

		buf++;
		len--;
		info->chars++;
	}
}

static int count_fd(const char *fn, int fd, struct count_info *info)
{
	ssize_t rrc;
	int rc = 0;
	mbstate_t ps;
	int mb = (MB_CUR_MAX > 1);
	char *buf = wc_buf;

	memset(&ps, 0, sizeof(ps));
	memset(info, 0, sizeof(*info));

	while (1) {
		rrc = read(fd, buf, sizeof(wc_buf) - (wc_buf - buf));
		if (rrc < 0) {
			perror(fn);
			return 1;
		}
		if (rrc == 0)
			break;

		if (!mb)
			count_buf(info, wc_buf, rrc);
		else {
			unsigned int dist;
			const char *src;
			size_t n_wc;

			/* convert bytes to wide char string */
			src = wc_buf;
			n_wc = mbsrtowcs(wc_buf_mb, &src, rrc, &ps);

			/* iterate over wide char string */
			count_buf_mb(info, wc_buf_mb, n_wc);

			/* handle any leftover octets */
			dist = rrc - (src - buf);
			memmove(wc_buf, src, dist);
			buf = wc_buf + dist;
		}

		info->bytes += rrc;
	}

	if (info->in_word)
		info->words++;
	if (info->in_line)
		info->lines++;

	return rc;
}

static void print_info (const char *fn, struct count_info *info)
{
	char numstr[128], s[32];
	const char *std_in = "(standard input)";
	bool need_space;
	bool need_filename = (strcmp(fn, std_in) != 0);
	numstr[0] = 0;

	if (outmask & WC_LINE) {
		need_space = (need_filename || outmask & WC_WORD
					    || outmask & WC_CHAR
					    || outmask & WCT_BYTE);
		sprintf(s, "%lu%s", info->lines, need_space ? " " : "");
		strcat(numstr, s);
	}
	if (outmask & WC_WORD) {
		need_space = (need_filename || outmask & WC_CHAR
					    || outmask & WCT_BYTE);
		sprintf(s, "%lu%s", info->words, need_space ? " " : "");
		strcat(numstr, s);
	}
	if (outmask & WC_CHAR) {
		need_space = need_filename;
		if (out_type == WCT_BYTE)
			sprintf(s, "%lu%s", info->bytes,
						need_filename ? " " : "");
		else
			sprintf(s, "%lu%s", info->chars,
						need_filename ? " " : "");
		strcat(numstr, s);
	}

	printf("%s%s\n", numstr, need_filename ? fn : "");
}

static int wc_actor(struct walker *w, const char *fn, int fd)
{
	struct count_info info;
	int rc;

	if (!outmask)
		outmask = WC_ALL;

	rc = count_fd(fn, fd, &info);
	if (rc)
		return rc;

	totals.bytes += info.bytes;
	totals.words += info.words;
	totals.lines += info.lines;
	total_files++;

	print_info(fn, &info);

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 'c':
		outmask |= WC_CHAR;
		out_type = WCT_BYTE;
		break;
	case 'm':
		outmask |= WC_CHAR;
		out_type = WCT_CHAR;
		break;
	case 'l':
		outmask |= WC_LINE;
		break;
	case 'w':
		outmask |= WC_WORD;
		break;

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int wc_post_walk(struct walker *w)
{
	if (total_files > 1)
		print_info(_("total"), &totals);

	return w->exit_status;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
