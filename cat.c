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
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("cat - concatenate files and print on the standard output");

static struct argp_option options[] = {
	{ NULL, 'u', NULL, 0, N_("(ignored)") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static int cat_fd(struct walker *w, const char *fn, int fd);

static const struct argp argp = { options, parse_opt, file_args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.cmdline_fd		= cat_fd,
};


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_IGNORE('u')
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int cat_fd(struct walker *w, const char *fn, int fd)
{
	ssize_t rc = copy_fd(STDOUT_NAME, STDOUT_FILENO, fn, fd);

	if (rc < 0)
		return 1;
	return 0;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

