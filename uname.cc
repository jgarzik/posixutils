
/*
 * Copyright 2008-2009 Red Hat, Inc.
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
#include <sys/utsname.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <libpu.h>


static const char doc[] =
N_("uname - return system name");

static struct argp_option options[] = {
	{ "all", 'a', NULL, 0,
	  N_("Behave as though all of the options -mnrsv were specified.") },
	{ "machine", 'm', NULL, 0,
	  N_("Write the name of the hardware type on which the system is running to standard output.") },
	{ "nodename", 'n', NULL, 0,
	  N_("Write the name of this node within an implementation-defined communications network.") },
	{ "kernel-release", 'r', NULL, 0,
	  N_("Write the current release level of the operating system implementation.") },
	{ "kernel-name", 's', NULL, 0,
	  N_("Write the name of the implementation of the operating system.") },
	{ "kernel-version", 'v', NULL, 0,
	  N_("Write the current version level of this release of the operating system implementation.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, NULL, doc };

enum parse_options_bits {
	OPT_MACHINE		= (1 << 1),
	OPT_NODE		= (1 << 2),
	OPT_KREL		= (1 << 3),
	OPT_KNAME		= (1 << 4),
	OPT_KVER		= (1 << 5),

	OPT_ALL			= (OPT_MACHINE | OPT_NODE | OPT_KREL |
				   OPT_KNAME | OPT_KVER),
};

static int opt_mask;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'a': opt_mask |= OPT_ALL; break;
	case 'm': opt_mask |= OPT_MACHINE; break;
	case 'n': opt_mask |= OPT_NODE; break;
	case 'r': opt_mask |= OPT_KREL; break;
	case 's': opt_mask |= OPT_KNAME; break;
	case 'v': opt_mask |= OPT_KVER; break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	error_t rc;
	char outstr[1024];
	struct utsname uts;

	pu_init();

	rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (rc) {
		fprintf(stderr, "argp_parse failed: %s\n", strerror(rc));
		return 1;
	}

	if (!opt_mask)
		opt_mask = OPT_KNAME;

	if (uname(&uts) < 0) {
		perror("uname");
		return 1;
	}

	outstr[0] = 0;

	if (opt_mask & OPT_MACHINE) {
		strcat(outstr, uts.machine);
		strcat(outstr, " ");
	}
	if (opt_mask & OPT_NODE) {
		strcat(outstr, uts.nodename);
		strcat(outstr, " ");
	}
	if (opt_mask & OPT_KREL) {
		strcat(outstr, uts.release);
		strcat(outstr, " ");
	}
	if (opt_mask & OPT_KNAME) {
		strcat(outstr, uts.sysname);
		strcat(outstr, " ");
	}
	if (opt_mask & OPT_KVER) {
		strcat(outstr, uts.version);
		strcat(outstr, " ");
	}
	if (outstr[0])
		outstr[strlen(outstr) - 1] = '\n';

	printf("%s", outstr);

	return 0;
}

