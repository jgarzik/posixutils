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

#define _GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libpu.h>

/* include auto-generated, platform-dependent signal list */
#include "kill-siglist.h"


static void usage(const char *progname)
{
	fprintf(stderr,
		"Usage:\n"
		"    %s -l [exit-status]\n"
		"    %s -s signal_name pid...\n"
		"    %s -signal_name pid...\n"
		"    %s -signal_number pid...\n",
		progname,
		progname,
		progname,
		progname);
	exit(1);
}

static int valid_signum(int signum)
{
	if ((signum < 0) || (signum > ARRAY_SIZE(platform_siglist)))
		return 0;	/* not valid */
	return 1;		/* valid */
}

static int signame_to_num(const char *signame)
{
	int i = 0, recurse = 0, rc, tmp = -1;
	const char *s;

	/* parse -[number] */
	rc = sscanf(signame, "%d", &tmp);
	if ((rc == 1) && (valid_signum(tmp)))
		return tmp;

	/* lookup signal name */
restart:
	while (i < ARRAY_SIZE(platform_siglist)) {
		s = platform_siglist[i];
		if (!s)
			goto loop;

		if (!strcasecmp(signame, platform_siglist[i]))
			return i;

loop:
		i++;
	}

	/* name not found. check alias list. */
	if (!recurse) {
		recurse = 1;

		for (i = 0; i < ARRAY_SIZE(platform_sigaliases); i += 2) {
			if (!strcasecmp(signame, platform_sigaliases[i])) {
				signame = platform_sigaliases[i + 1];
				goto restart;
			}
		}
	}

	return -1;
}

static void list_signal(int signum)
{
	const char *s = platform_siglist[signum];
	if (s)
		printf("%d) SIG%s\n", signum, s);
}

static void list_signals(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(platform_siglist); i++)
		list_signal(i);

	if (ARRAY_SIZE(platform_sigaliases) < 1)
		return;

	printf("\nAliases:\n");

	for (i = 0; i < ARRAY_SIZE(platform_sigaliases); i += 2)
		printf("SIG%s -> SIG%s\n",
		       platform_sigaliases[i],
		       platform_sigaliases[i + 1]);
}

static void check_signal_list(int argc, char **argv)
{
	int rc, tmp;

	/* -l [exit-status] */
	if (strcmp(argv[1], "-l"))
		return;

	if (argc == 2) {
		list_signals();
		exit(0);
	}
	if (argc > 3)
		usage(argv[0]);

	rc = sscanf(argv[2], "%d", &tmp);
	if ((rc != 1) || (!valid_signum(tmp)))
		usage(argv[0]);

	list_signal(tmp);
	exit(0);
}

static void check_help(char **argv)
{
	if (!strcmp(argv[1], "-h") ||
	    !strcmp(argv[1], "-?") ||
	    !strcmp(argv[1], "--help"))
		usage(argv[0]);
}

static int check_signal_name(int argc, char **argv, int *pidpos_out)
{
	char *name;
	int signum;

	if ((argc < 3) && (argv[1][0] == '-'))
		usage(argv[0]);

	if (!strcmp(argv[1], "-s")) {
		name = argv[2];
		*pidpos_out = 3;

		if (argc < 4)
			usage(argv[0]);
	} else if (argv[1][0] == '-') {
		name = argv[1] + 1;
		*pidpos_out = 2;
	} else {
		return SIGTERM;		/* default */
	}

	signum = signame_to_num(name);
	if (signum < 0) {
		fprintf(stderr, "invalid signal '%s'\n", name);
		exit(1);
	}

	return signum;
}

static int deliver_signals(int argc, char **argv, int pidpos, int signum)
{
	int i, retval = 0;

	for (i = pidpos; i < argc; i++) {
		int rc, tmp;

		tmp = 0;
		rc = sscanf(argv[i], "%d", &tmp);
		if ((rc == 1) && (tmp >= 0)) {
			if (kill(tmp, signum) < 0) {
				perror(argv[i]);
				retval = 1;
			}
		} else {
			fprintf(stderr, "invalid pid '%s'\n", argv[i]);
			retval = 1;
		}
	}

	return retval;
}

int main (int argc, char *argv[])
{
	int signum, pidpos;

	pu_init();

	if (argc < 2)
		usage(argv[0]);

	check_help(argv);
	check_signal_list(argc, argv);

	pidpos = -1;
	signum = check_signal_name(argc, argv, &pidpos);

	return deliver_signals(argc, argv, pidpos, signum);
}

