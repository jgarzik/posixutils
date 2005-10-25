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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <libpu.h>


static void sigalarm(int signal)
{
	exit(0);
}

int main (int argc, char *argv[])
{
	int rc, sleep_time = 0;
	unsigned int seconds;

	pu_init();

	if (signal(SIGALRM, sigalarm) == SIG_ERR) {
		fprintf(stderr, _("cannot install SIGALARM handler\n"));
		return 1;
	}

	if (argc != 2) {
		fprintf(stderr, _("invalid arguments\n"));
		return 1;
	}

	rc = sscanf(argv[1], "%d", &sleep_time);
	if ((rc != 1) || (sleep_time < 0)) {
		fprintf(stderr, _("invalid sleep argument\n"));
		return 1;
	}

	seconds = (unsigned int) sleep_time;
	do {
		seconds = sleep(seconds);
	} while (seconds > 0);

	return 0;
}
