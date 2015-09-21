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
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <argp.h>
#include <pwd.h>
#include <libpu.h>


static const char doc[] =
N_("renice - alter priority of running processes");

static const char args_doc[] = N_("ID ...");

static struct argp_option options[] = {
	{ NULL, 'g', NULL, 0,
	  N_("Interpret all operands as unsigned decimal integer process group IDs") },
	{ NULL, 'p', NULL, 0,
	  N_("Interpret all operands as unsigned decimal integer process IDs. The -p option is the default if no options are specified") },
	{ NULL, 'u', NULL, 0,
	  N_("Interpret all operands as users.") },
	{ NULL, 'n', "increment", 0,
	  N_("Specify how the nice value of the specified process or processes is to be adjusted") },
	{ }
};

static int renice_actor(struct walker *w, const char *fn, const struct stat *lst);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker;

static int nice_increment;
static int nice_mode = PRIO_PROCESS;


static int renice_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	int rc, old_prio, prio, valid_val, val = 0;

	rc = sscanf(fn, "%d", &val);
	valid_val = (rc == 1);

	if (nice_mode == PRIO_USER) {
		struct passwd *pw = getpwnam(fn);
		if (pw)
			val = pw->pw_uid;
		else if (!valid_val) {
			perror(fn);
			return 1;
		}
	}

	old_prio = getpriority(nice_mode, val);
	if (old_prio < 0) {
		fprintf(stderr, "getpriority(%s): %s\n", fn, strerror(errno));
		return 1;
	}

	if (nice_increment) {
		/* NOTE: Linux renice interprets "increment"
		 * as absolute value
		 */
		int new_prio = old_prio + nice_increment;
		prio = setpriority(nice_mode, val, new_prio);
		if (prio < 0) {
			fprintf(stderr, "setpriority(%s -> %d): %s\n",
				fn, new_prio, strerror(errno));
			return 1;
		}

		prio = getpriority(nice_mode, val);
		if (prio < 0) {
			fprintf(stderr, "getpriority 2(%s): %s\n",
				fn, strerror(errno));
			return 1;
		}
	} else {
		prio = old_prio;
	}

	/* NOTE: not POSIX correct, but I like the output.  Maybe
	 * we'll kill this...
	 */
	printf("%s: old priority %d, new priority %d\n",
	       fn, old_prio, prio);
	return 1;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_ARG

	case 'n': {
		int rc, tmp = 0;

		rc = sscanf(arg, "%d", &tmp);
		if ((rc != 1) || (tmp < PRIO_MIN) || (tmp > PRIO_MAX))
			return ARGP_ERR_UNKNOWN;
		nice_increment = tmp;
		break;
	}

	case 'p':
		nice_mode = PRIO_PROCESS;
		break;
	case 'g':
		nice_mode = PRIO_PGRP;
		break;
	case 'u':
		nice_mode = PRIO_USER;
		break;

	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.cmdline_arg		= renice_actor;
	return walk(&walker, argc, argv);
}

