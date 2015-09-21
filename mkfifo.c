
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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("mkfifo - make FIFOs (named pipes)");

static struct argp_option options[] = {
	{ }
};

static error_t args_parse_opt (int key, char *arg, struct argp_state *state);
static int mkfifo_arg(struct walker *w, const char *fn, const struct stat *lst);

static const struct argp argp = { options, args_parse_opt, file_args_doc, doc };

static struct walker walker;

DECLARE_PU_PARSE_ARGS

static int mkfifo_arg(struct walker *w, const char *fn, const struct stat *lst)
{
	/* TODO: handle -m mode argument, using chmod(1)-like mode parser */
	if (mkfifo(fn, 0666) < 0) {
		perror(fn);
		w->exit_status = 1;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.cmdline_arg		= mkfifo_arg;
	return walk(&walker, argc, argv);
}

