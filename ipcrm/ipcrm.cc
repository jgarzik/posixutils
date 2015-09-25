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
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <vector>
#include <argp.h>
#include <libpu.h>

using namespace std;


static const char doc[] =
N_("ipcrm - remove an XSI message queue, semaphore set, or shared memory segment identifier");

static struct argp_option options[] = {
	{ NULL, 'q', "msgid", 0,
	  N_("Remove message queue identifier msgid from system") },
	{ NULL, 'm', "shmid", 0,
	  N_("Remove shared memory identifier shmid from system") },
	{ NULL, 's', "semid", 0,
	  N_("Remove semaphore identifier semid from system") },
	{ NULL, 'Q', "msgkey", 0,
	  N_("Remove message queue identifier, created with key msgkey, from system") },
	{ NULL, 'M', "shmkey", 0,
	  N_("Remove shared memory identifier, created with key shmkey, from system") },
	{ NULL, 'S', "semkey", 0,
	  N_("Remove semaphore identifier, created with key semkey, from system") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, NULL, doc };

enum parse_options_bits {
	OPT_MSG			= (1 << 0),
	OPT_SHM			= (1 << 1),
	OPT_SEM			= (1 << 2),
	OPT_KEY			= (1 << 3),
};

class argent {
public:
	int			mask;
	unsigned long		arg;

	argent(int mask_, unsigned long arg_) : mask(mask_), arg(arg_) {
	}
};

#ifdef _SEM_SEMUN_UNDEFINED
   union semun
   {
     int val;
     struct semid_ds *buf;
     unsigned short int *array;
     struct seminfo *__buf;
   };
#endif

static int exit_status = EXIT_SUCCESS;
static vector<argent> arglist;


static const char *arg_name(int mask)
{
	if (mask & OPT_MSG) return "msg";
	if (mask & OPT_SHM) return "shm";
	if (mask & OPT_SEM) return "sem";
	return NULL;
}

static void push_opt(int mask, unsigned long arg)
{
	argent ae(mask, arg);
	arglist.push_back(ae);
}

static void push_arg_opt(int mask, const char *arg)
{
	int base = (mask & OPT_KEY) ? 0 : 10;
	char *end = NULL;
	unsigned long l;

	l = strtoul(arg, &end, base);

	if ((*end != 0) ||	/* entire string is -not- valid */
	    ((mask & OPT_KEY) && (l == IPC_PRIVATE))) {
		fprintf(stderr, "%s%s '%s' invalid\n",
			arg_name(mask),
			mask & OPT_KEY ? "key" : "id",
			arg);
		exit_status = EXIT_FAILURE;
		return;
	}

	push_opt(mask, l);
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'q': push_arg_opt(OPT_MSG, arg); break;
	case 'm': push_arg_opt(OPT_SHM, arg); break;
	case 's': push_arg_opt(OPT_SEM, arg); break;
	case 'Q': push_arg_opt(OPT_MSG | OPT_KEY, arg); break;
	case 'M': push_arg_opt(OPT_SHM | OPT_KEY, arg); break;
	case 'S': push_arg_opt(OPT_SEM | OPT_KEY, arg); break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static void pinterr(const char *msg, long l)
{
	fprintf(stderr, msg, l, strerror(errno));
	exit_status = EXIT_FAILURE;
}

static void remove_one(const argent& ae)
{
	int rc;
	int id = (int) ae.arg;
	const char *errmsg = NULL;

	if (ae.mask & OPT_KEY) {
		if (ae.mask & OPT_MSG)
			id = msgget(ae.arg, 0);
		else if (ae.mask & OPT_SHM)
			id = shmget(ae.arg, 0, 0);
		else if (ae.mask & OPT_SEM)
			id = semget(ae.arg, 0, 0);
		else
			abort();	/* should never happen */
	}

	if (id < 0) {
		pinterr("key 0x%lx lookup failed: %s\n", ae.arg);
		return;
	}

	if (ae.mask & OPT_MSG) {
		rc = msgctl(id, IPC_RMID, NULL);
		errmsg = "msgctl(0x%x): %s\n";
	}
	else if (ae.mask & OPT_SHM) {
		rc = shmctl(id, IPC_RMID, NULL);
		errmsg = "shmctl(0x%x): %s\n";
	}
	else if (ae.mask & OPT_SEM) {
		union semun dummy;
		dummy.val = 0;

		rc = semctl(id, 0, IPC_RMID, dummy);
		errmsg = "semctl(0x%x): %s\n";
	}

	else
		abort();	/* should never happen */

	if (rc < 0) {
		fprintf(stderr, errmsg, id, strerror(errno));
		exit_status = EXIT_FAILURE;
	}
}

int main (int argc, char *argv[])
{
	error_t rc;

	pu_init();

	// parse command line
	rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (rc) {
		fprintf(stderr, "argp_parse failed: %s\n", strerror(rc));
		return 1;
	}

	// remove entities as requested via CLI options
	for (vector<argent>::iterator it = arglist.begin();
	     it != arglist.end(); ++it)
		remove_one(*it);

	return exit_status;
}

