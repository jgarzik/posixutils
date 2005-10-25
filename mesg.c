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

int main (int argc, char *argv[])
{
	int i, ttyfd = -1;
	const char *ttyname;
	struct stat st;

	pu_init();

	if ((argc > 2) ||
	    ((argc == 2) && (!strcmp(argv[1], "y")) && (!strcmp(argv[1], "n")))) {
		fprintf(stderr, _("invalid arguments\n"));
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
	if (argc == 1) {
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
	if (!strcmp(argv[1], "y")) {
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

