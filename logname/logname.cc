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
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("logname - return the user's login name");

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static const struct argp argp = { NULL, parse_opt, NULL, doc };

int main (int argc, char *argv[])
{
	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, "%s: argp_parse failed: %s\n",
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	char *s = getlogin();
	if (!s) {
		perror(_("getlogin(3)"));
		return 1;
	}

	return (printf("%s\n", s) < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}

