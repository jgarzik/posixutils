
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

#ifdef __APPLE__
#define _DARWIN_C_SOURCE 1
#endif

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string>
#include <libpu.h>


static const char doc[] =
N_("getconf - get configuration values");

static const char args_doc[] = N_("[system_var | path_var pathname]");

static struct argp_option options[] = {
	{ NULL, 'v', "specification", 0,
	  N_("not implemented yet") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };


static const char *opt_var;
static const char *opt_pathname;
static const char *opt_spec;


#undef STRMAP

#define STRMAP pathvar_map
#include "getconf-path.h"
#undef STRMAP

#define STRMAP sysvar_map
#include "getconf-system.h"
#undef STRMAP

#define STRMAP confstr_map
#include "getconf-confstr.h"
#undef STRMAP

static int do_var(const struct strmap *map, bool pathvar)
{
	int val;
	long l;

	val = map_lookup(map, opt_var);
	if (val < 0) {
		if (pathvar)
			fprintf(stderr, _("invalid path variable %s\n"), opt_var);
		else
			fprintf(stderr, _("invalid system variable %s\n"), opt_var);
		return 1;
	}

	errno = 0;
	if (pathvar) {
		l = pathconf(opt_pathname, val);
		if (l < 0) {
			perror(_("pathconf(3)"));
			return 1;
		}
	} else {
		l = sysconf(val);
		if (l < 0) {
			perror(_("sysconf(3)"));
			return 1;
		}
	}

	printf("%ld\n", l);
	return 0;
}

static int do_confstr(int val)
{
	size_t n = confstr(val, NULL, 0);
	if (n == 0) {
		fprintf(stderr, _("invalid confstr variable %s\n"), opt_var);
		return 1;
	}

	n++;				// C nul
	std::string buf(n, '\0');
	confstr(val, &buf[0], n);

	printf("%s\n", buf.c_str());

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'v':
		opt_spec = arg;
		break;

	case ARGP_KEY_ARG:
		switch(state->arg_num) {
		case 0:
			opt_var = arg;
			break;
		case 1:
			opt_pathname = arg;
			break;

		default:			/* too many args */
			argp_usage (state);
			break;
		}
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 1)		/* not enough args */
			argp_usage (state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	int val;

	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	/* if we have a pathname argument, we use pathconf(3) */
	if (opt_pathname)
		return do_var(pathvar_map, true);

	/* if it's in the confstr list, use confstr(3) */
	val = map_lookup(confstr_map, opt_var);
	if (val >= 0)
		return do_confstr(val);

	/* otherwise, use the sysconf(3) variable list */
	return do_var(sysvar_map, false);
}

