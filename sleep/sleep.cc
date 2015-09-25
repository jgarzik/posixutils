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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("sleep - suspend execution for an interval");

static const char args_doc[] = N_("time");

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { NULL, parse_opt, args_doc, doc };
static char *opt_seconds = NULL;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case ARGP_KEY_ARG:
		if (state->arg_num == 0)
			opt_seconds = arg;
		else
			argp_usage(state);
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 1)         /* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static void sigalarm(int signal)
{
	exit(EXIT_SUCCESS);
}

int main (int argc, char *argv[])
{
	unsigned int seconds = 0;

	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	if (signal(SIGALRM, sigalarm) == SIG_ERR) {
		fprintf(stderr, _("cannot install SIGALRM handler\n"));
		return EXIT_FAILURE;
	}

	int rc = sscanf(opt_seconds, "%u", &seconds);
	if (rc != 1) {
		fprintf(stderr, _("invalid sleep time argument\n"));
		return EXIT_FAILURE;
	}

	do {
		seconds = sleep(seconds);
	} while (seconds > 0);

	return EXIT_SUCCESS;
}
