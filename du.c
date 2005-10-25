
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
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <libpu.h>


static const char doc[] =
N_("du - estimate file space usage");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ NULL, 'a', NULL, 0,
	  N_("Report file sizes in addition to directory totals.") },
	{ NULL, 'H', NULL, 0,
	  N_("If a symbolic link is specified on the command line, du shall count the size of the file or file hierarchy referenced by the link") },
	{ NULL, 'k', NULL, 0,
	  N_("Write the files sizes in units of 1024 bytes, rather than the default 512-byte units.") },
	{ NULL, 'L', NULL, 0,
	  N_("If a symbolic link is specified on the command line or encountered during the traversal of a file hierarchy, du shall count the size of the file or file hierarchy referenced by the link.") },
	{ NULL, 's', NULL, 0,
	  N_("Instead of the default output, report only the total sum for each of the specified files.") },
	{ NULL, 'x', NULL, 0,
	  N_("When evaluating file sizes, evaluate only those files that have the same device as the file specified by the file operand.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static bool opt_show_files;
static bool opt_follow_link_cmdline;
static bool opt_1024;
static bool opt_follow_link;
static bool opt_sum_only;
static bool opt_one_filesys;


#define OPT(key, var) \
	case (key): opt_##var = true; break;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	OPT('a', show_files)
	OPT('H', follow_link_cmdline)
	OPT('k', 1024)
	OPT('L', follow_link)
	OPT('s', sum_only)
	OPT('x', one_filesys)
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int du_actor(struct cmdline_walker *cw, const char *fn)
{
	return 1; /* TODO */
}

int main (int argc, char *argv[])
{
	PU_FN_WALKER(du_actor, 0)
}

