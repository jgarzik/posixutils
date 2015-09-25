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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <assert.h>
#include <libpu.h>


static const char doc[] =
N_("nice - run a program with modified scheduling priority");

static const char args_doc[] = N_("command [arg...]");

static struct argp_option options[] = {
	{ "adjustment", 'n', "increment", 0,
	  N_("Positive or negative integer indicating desired priority adjustment") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

enum random_constants {
	NICE_MIN	= -30,
	NICE_MAX	= 30,
	NICE_DEF	= 10
};

static int nice_inc = NICE_DEF;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'n': {
		int tmp;
		if (sscanf(arg, "%d", &tmp) != 1)
			return ARGP_ERR_UNKNOWN;
		if ((tmp < NICE_MIN) || (tmp > NICE_MAX))
			return ARGP_ERR_UNKNOWN;
		nice_inc = tmp;
		break;
	}

	case ARGP_KEY_END:
		if (state->arg_num < 1)		/* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	char **nice_args;
	int n_args, i, idx;

	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, &idx, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	if (nice(nice_inc) < 0) {
		perror(_("nice(2)"));
		exit(1);
	}

	n_args = argc - idx;
	assert(n_args >= 1);

	nice_args = (char **) xmalloc((n_args + 1) * sizeof(char *));

	for (i = 0; i < n_args; i++)
		nice_args[i] = argv[idx + i];
	nice_args[n_args] = NULL;

	execvp(argv[idx], nice_args);

	/* if we return, we had an error */
	perror(_("execvp(3)"));
	return 1;
}

