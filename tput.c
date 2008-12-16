
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


static const char doc[] =
N_("tput - change terminal characteristics");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "term", 'T', "type", 0,
	  N_("Indicate the type of terminal.") },
	{ }
};

static int tput_actor(struct walker *w, const char *fn, const struct stat *lst);
static int tput_pre_walk(struct walker *w);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static char *opt_term;

static struct walker walker = {
	.argp			= &argp,
	.cmdline_arg		= tput_actor,
	.pre_walk		= tput_pre_walk,
};

static char *xtigetstr(char *capname)
{
	char *s = tigetstr(capname);
	if (s == (char *)-1)
		s = NULL;
	
	return s;
}

static void tput_clear(void)
{
	tputs(clear_screen, lines > 0 ? lines : 1, putchar);
}

static char *init_names[] = {
	"iprog",
	"is1",
	"is2",
	"if",
	"is3",
};

static char *reset_names[] = {
	"rprog",
	"rs1",
	"rs2",
	"rf",
	"rs3",
};

static void tput_reset(bool is_init)
{
	char **caps = is_init ? init_names : reset_names;
	char *val;

	val = xtigetstr(caps[0]);
	if (val)
		system(val);

	val = xtigetstr(caps[1]);
	if (val)
		write(STDOUT_FILENO, val, strlen(val));

	val = xtigetstr(caps[2]);
	if (val)
		write(STDOUT_FILENO, val, strlen(val));

	val = xtigetstr(caps[3]);
	if (val) {
		FILE *f;

		f = fopen(val, "r");
		if (f) {
			int ch;
			char chv;

			while ((ch = getc(f)) != EOF) {
				chv = ch;
				write(STDOUT_FILENO, &ch, 1);
			}

			fclose(f);
		}
	}

	val = xtigetstr(caps[4]);
	if (val)
		write(STDOUT_FILENO, val, strlen(val));

}

static int tput_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	if (!strcmp(fn, "clear"))
		tput_clear();
	else if (!strcmp(fn, "init"))
		tput_reset(true);
	else if (!strcmp(fn, "reset"))
		tput_reset(false);

	return 0;
}

static int tput_pre_walk(struct walker *w)
{
	int err = 0;

	if (setupterm(opt_term, STDOUT_FILENO, &err) != OK) {
		fprintf(stderr, _("tput: setupterm failed\n"));
		return 1;
	}

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 'T':
		opt_term = arg;
		break;

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	opt_term = get_terminal();
	return walk(&walker, argc, argv);
}

