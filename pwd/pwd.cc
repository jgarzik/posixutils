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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE		/* for O_DIRECTORY */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libpu.h>


static const char doc[] =
N_("pwd - print name of current/working directory");

static struct argp_option options[] = {
	{ NULL, 'L', NULL, 0,
	  N_("If the PWD environment variable contains an absolute pathname of the current directory that does not contain the filenames dot or dot-dot, pwd shall write this pathname to standard output. Otherwise, the -L option shall behave as the -P option.") },
	{ NULL, 'P', NULL, 0,
	  N_("The absolute pathname written shall not contain filenames that, in the context of the pathname, refer to files of type symbolic link.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, NULL, doc };


enum pwd_mode_t {
	MODE_ENV,
	MODE_GETCWD,
};

static int opt_mode = MODE_ENV;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'L':
		opt_mode = MODE_ENV;
		break;
	case 'P':
		opt_mode = MODE_GETCWD;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int pwd_valid(const char *path)
{
	int rc;

	if (!strcmp(path, "/"))
		return 1;
	if (have_dots(path))
		return 0;

	pathelem pe;
	path_split(path, pe);
	if (have_dots(pe.basen.c_str())) {
		rc = 0;
		goto out;
	}

	rc = pwd_valid(pe.dirn.c_str());

out:
	return rc;
}

static char *xgetcwd(void)
{
	int len = 128;
	char *cwd = NULL;
	char *cwd_ret = NULL;

	while (cwd_ret == NULL) {
		len <<= 1;
		cwd = (char *) xrealloc(cwd, len);

		cwd_ret = getcwd(cwd, len);
		if ((cwd_ret == NULL) && (errno != ERANGE)) {
			perror(_("getcwd(3) failed"));
			exit(1);
		}
	}

	return cwd;
}

int main (int argc, char *argv[])
{
	const char *pwd;

	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	if ((opt_mode == MODE_ENV) && ((pwd = getenv("PWD")) != NULL) &&
	    (pwd_valid(pwd))) {
		/* do nothing */
	} else
		pwd = xgetcwd();

	printf("%s\n", pwd);
	return 0;
}

