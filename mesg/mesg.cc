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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libpu.h>


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct {
	int fd;
	const char *name;
} stdio_fds[] = {
	{ STDIN_FILENO, STDIN_NAME },
	{ STDOUT_FILENO, STDOUT_NAME },
	{ STDERR_FILENO, STDERR_NAME }
};

static const char doc[] =
N_("mesg - permit or deny messages");

static const char args_doc[] = N_("[y|n]");

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { NULL, parse_opt, args_doc, doc };
static char *opt_yn = NULL;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) {
			if (strcmp(arg, "y") && strcmp(arg, "n"))
				argp_usage(state);
			else
				opt_yn = arg;
		} else
			argp_usage(state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	unsigned int i;
	int ttyfd = -1;
	const char *ttyname;
	struct stat st;

	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return 2;
	}

	ttyname = NULL;
	for (i = 0; i < ARRAY_SIZE(stdio_fds); i++)
		if (isatty(stdio_fds[i].fd)) {
			ttyfd = i;
			ttyname = stdio_fds[i].name;
			break;
		}

	if (ttyfd == -1) {
		fprintf(stderr, _("no terminal device found\n"));
		return 2;
	}

	if (fstat(ttyfd, &st) < 0) {
		perror(ttyname);
		return 2;
	}

	/* no arguments, just print out current tty perm state */
	if (!opt_yn) {
		if (st.st_mode & (S_IWGRP | S_IWOTH)) {
			printf(_("is y\n"));
			i = 0;
		} else {
			printf(_("is n\n"));
			i = 1;
		}
		return i;
	}

	/* mesg = y */
	if (!strcmp(opt_yn, "y")) {
		if (st.st_mode & (S_IWGRP | S_IWOTH))
			return 0;

		st.st_mode |= S_IWGRP | S_IWOTH;
		if (fchmod(ttyfd, st.st_mode) < 0) {
			perror(ttyname);
			return 2;
		}

		return 0;
	}

	/* mesg = n */
	if ((st.st_mode & (S_IWGRP | S_IWOTH)) == 0)
		return 1;

	st.st_mode &= ~(S_IWGRP | S_IWOTH);
	if (fchmod(ttyfd, st.st_mode) < 0) {
		perror(ttyname);
		return 2;
	}

	return 1;
}

