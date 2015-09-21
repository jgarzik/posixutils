
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <term.h>
#include <libpu.h>

enum {
	tabs_outbuf_sz		= 4096,
	max_tab_stops		= 1024,
};

static char *opt_term;
static int n_tabs;
static int tab_stop[max_tab_stops];

static const int tabset_a[] = { 1,10,16,36,72 };
static const int tabset_a2[] = { 1,10,16,40,72 };
static const int tabset_c[] = { 1,8,12,16,20,55 };
static const int tabset_c2[] = { 1,6,10,14,49 };
static const int tabset_c3[] = { 1,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,67 };
static const int tabset_f[] = { 1,7,11,15,19,23 };
static const int tabset_p[] = { 1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61 };
static const int tabset_s[] = { 1,10,55 };
static const int tabset_u[] = { 1,12,20,44 };

static void usage(void)
{
	fprintf(stderr,
"Usage:\n"
"tabs [ -n| -a| -a2| -c| -c2| -c3| -f| -p| -s| -u][+m[n]] [-T type]\n"
"        or\n"
"tabs [-T type][ +[n]] n1[,n2,...]\n");
	exit(1);
}

static void tab_push(int stop)
{
	if (n_tabs == max_tab_stops) {
		fprintf(stderr, _("tabs: tab stop table limit reached\n"));
		exit(1);
	}

	tab_stop[n_tabs++] = stop;
}

#define tab_stock(arr) __tab_stock(arr, ARRAY_SIZE(arr))

static void __tab_stock(const int *stops, int n_stops)
{
	n_tabs = n_stops;
	memcpy(tab_stop, stops, n_stops * sizeof(int));
}

static void tab_repeating(int tab_n)
{
	int col;

	for (col = 1; col < columns; col++)
		if (col % tab_n == 0)
			tab_push(col);
}

static void cmdline(int argc, char **argv)
{
	int opt, i, col = 1, err = 0;
	bool changed = false;
	char tmparg[4096], *tok, *tmp_s;
	const int skip = 2;	/* skip -T */

	/* get terminal type immediately, so that calculations
	 * based on terminal size, needed during argument parsing,
	 * will succeed.
	 */
	for (i = 1; i < argc; i++)
		if (strncmp(argv[i], "-T", 2) == 0) {
			if (*(argv[i] + skip) == '\0') {
				i++;
				if (i < argc)
					opt_term = argv[i];
			} else {
				opt_term = argv[i] + skip;
			}
		}

	if (setupterm(opt_term, STDOUT_FILENO, &err) != OK) {
		fprintf(stderr, _("tabs: setupterm failed for term type %s\n"),
			(opt_term && *opt_term) ? opt_term : _("(null)"));
		exit(1);
	}

	tab_repeating(8);

	while ((opt = getopt(argc, argv, "0123456789a::c::fpsuT:")) != -1) {
		switch (opt) {
		case '0':
			n_tabs = 0;
			changed = true;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			tab_repeating(opt - '0');
			changed = true;
			break;

		case 'a':
			changed = true;
			if (!optarg)
				tab_stock(tabset_a);
			else if (!strcmp(optarg, "2"))
				tab_stock(tabset_a2);
			else
				usage();
			break;

		case 'c':
			changed = true;
			if (!optarg)
				tab_stock(tabset_c);
			else if (!strcmp(optarg, "2"))
				tab_stock(tabset_c2);
			else if (!strcmp(optarg, "3"))
				tab_stock(tabset_c3);
			else
				usage();
			break;

		case 'f': changed = true; tab_stock(tabset_f); break;
		case 'p': changed = true; tab_stock(tabset_p); break;
		case 's': changed = true; tab_stock(tabset_s); break;
		case 'u': changed = true; tab_stock(tabset_u); break;

		case 'T':
			break;

		default:
			usage();
			break;
		}
	}

	if (optind == argc)
		return;

	if (changed)
		usage();

	n_tabs = 0;

	while (optind < argc) {
		bool do_offset;
		int offset, last_col = -1;

		strcpy(tmparg, argv[optind++]);
		tmp_s = tmparg;
		while ((tok = strtok(tmp_s, ", \r\n\f\t")) != NULL) {
			tmp_s = NULL;
			do_offset = false;
			if (*tok == '+') {
				tok++;
				do_offset = true;
			}
			offset = atoi(tok);

			if (do_offset) {
				if (offset > columns ||
				    offset > (col + columns)) {
					fprintf(stderr, _("tabs: tabspec beyond max-columns specified\n"));
					exit(1);
				}

				col += offset;
			} else {
				if (offset < col)
					usage();

				col = offset;
			}

			if (col == last_col)
				usage();

			tab_push(col);

			last_col = col;
		}
	}
}

static char outbuf[tabs_outbuf_sz];
static unsigned int outbuf_avail = tabs_outbuf_sz - 1;

static bool push_str(const char *val)
{
	size_t val_len;

	if (!val)
		return true;

	val_len = strlen(val);
	if (outbuf_avail < val_len)
		return false;
	
	strcat(outbuf, val);
	outbuf_avail -= val_len;

	return true;
}

static void set_hw_tabs(void)
{
	int i, col = 0;
	char *tab_set;

	tab_set = xtigetstr("hts");
	if (!tab_set) {
		fprintf(stderr, _("tabs: terminal unable to set tabs\n"));
		exit(1);
	}

	if (!push_str(xtigetstr("tbc")))
		goto err_out;

	for (i = 0; i < n_tabs; i++) {
		while (col < tab_stop[i]) {
			if (!push_str(" "))
				goto err_out;
			col++;
		}

		if (!push_str(tab_set))
			goto err_out;
	}

	write(STDOUT_FILENO, outbuf, strlen(outbuf));

	return;

err_out:
	fprintf(stderr, _("tabs: buffer overflow\n"));
	exit(1);
}

int main (int argc, char *argv[])
{
	pu_init();
	opt_term = get_terminal();
	cmdline(argc, argv);
	set_hw_tabs();

	return 0;
}
