
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

#define _BSD_SOURCE	/* for wait4(2) */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <libpu.h>

static struct timeval start_time;

static int do_child(int argc, char **argv)
{
	execvp(argv[0], argv);
	perror(_("exec failed"));
	return 126;
}

static int do_wait(pid_t pid)
{
	struct rusage ru;
	int status = 0;
	pid_t rc;
	struct timeval end_time;
	unsigned long long sec, usec;

	rc = wait4(pid, &status, 0, &ru);
	if (rc < 0) {
		perror(_("wait4(child)"));
		return 1;
	}

	if (WIFEXITED(status) && (WEXITSTATUS(status) == 126)) {
		fprintf(stderr, _("exec failed\n"));
		return 126;
	}

	if (gettimeofday(&end_time, NULL) < 0) {
		perror(_("gettimeofday 2"));
		return 1;
	}

	sec = end_time.tv_sec - start_time.tv_sec;
	if (end_time.tv_usec < start_time.tv_usec) {
		end_time.tv_usec += 1000000;
		sec--;
	}
	usec = end_time.tv_usec - start_time.tv_usec;

	fprintf(stderr,
		_("real %Lu.%Lu\n"
		"user %Lu.%Lu\n"
		"sys %Lu.%Lu\n"),
		sec,
		usec / 10000,
		(unsigned long long) ru.ru_utime.tv_sec,
		(unsigned long long) ru.ru_utime.tv_usec / 10000ULL,
		(unsigned long long) ru.ru_stime.tv_sec,
		(unsigned long long) ru.ru_stime.tv_sec / 10000ULL);

	return WEXITSTATUS(status);
}

int main (int argc, char *argv[])
{
	pid_t pid;
	int have_p;

	pu_init();

	have_p = (argc == 2) && (!strcmp("-p", argv[1]));

	if (argc == 1 || have_p) {
		fprintf(stderr, _("no args\n"));
		return 1;
	}

	if (gettimeofday(&start_time, NULL) < 0) {
		perror(_("gettimeofday"));
		return 1;
	}

	pid = fork();
	if (pid < 0) {
		perror(_("fork"));
		return 1;
	}
	if (pid == 0) {		/* child process */
		return do_child(have_p ? (argc - 2) : (argc - 1),
				 have_p ? (argv + 2) : (argv + 1));
	}

	return do_wait(pid);
}
