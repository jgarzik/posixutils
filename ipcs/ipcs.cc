
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE		/* for MSG_INFO, MSG_STAT */
#endif

#include <sys/types.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <libpu.h>


static const char doc[] =
N_("ipcs - report XSI interprocess communication facilities status");

static struct argp_option options[] = {
	{ "msg", 'q', NULL, 0,
	  N_("Write information about active message queues.") },
	{ "shm", 'm', NULL, 0,
	  N_("Write information about active shared memory segments.") },
	{ "sem", 's', NULL, 0,
	  N_("Write information about active semaphore sets.") },

	{ "all", 'a', NULL, 0,
	  N_("Use all print options. (This is a shorthand notation for -b, -c, -o, -p, and -t.)") },
	{ "max-size", 'b', NULL, 0,
	  N_("Write information on maximum allowable size.") },
	{ "creator", 'c', NULL, 0,
	  N_("Write creator's user name and group name") },
	{ "outstanding", 'o', NULL, 0,
	  N_("Write information on outstanding usage.") },
	{ "process", 'p', NULL, 0,
	  N_("Write process number information.") },
	{ "time", 't', NULL, 0,
	  N_("Write time information.") },

	{ }
};

/* X/Open says we must define this ourselves? */
union semun {
	int			val;
	struct semid_ds		*buf;
	unsigned short		*array;
	struct seminfo		*__buf;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, NULL, doc };

enum parse_options_ibits {
	IOPT_MSG		= (1 << 0),
	IOPT_SHM		= (1 << 1),
	IOPT_SEM		= (1 << 2),
	IOPT_ALL		= IOPT_MSG | IOPT_SHM | IOPT_SEM,
};

enum parse_options_rbits {
	ROPT_SIZE		= (1 << 0),
	ROPT_CREATOR		= (1 << 1),
	ROPT_OUTST		= (1 << 2),
	ROPT_PROC		= (1 << 3),
	ROPT_TIME		= (1 << 4),

	ROPT_ALL		= ROPT_SIZE | ROPT_CREATOR | ROPT_OUTST |
				  ROPT_PROC | ROPT_TIME,
};

static unsigned int opt_info, opt_print;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'q': opt_info |= IOPT_MSG; break;
	case 'm': opt_info |= IOPT_SHM; break;
	case 's': opt_info |= IOPT_SEM; break;

	case 'a': opt_print |= ROPT_ALL; break;
	case 'b': opt_print |= ROPT_SIZE; break;
	case 'c': opt_print |= ROPT_CREATOR; break;
	case 'o': opt_print |= ROPT_OUTST; break;
	case 'p': opt_print |= ROPT_PROC; break;
	case 't': opt_print |= ROPT_TIME; break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int print_header(void)
{
	char timestr[64];
	time_t time_n;
	struct tm tm;
	struct utsname uts;

	if (uname(&uts) < 0) {
		perror("uname");
		return 1;
	}

	time_n = time(NULL);
	if (time_n == ((time_t)-1)) {
		perror("time");
		return 1;
	}

	localtime_r(&time_n, &tm);

	if (strftime(timestr, sizeof(timestr),
		     "%a, %d %b %Y %H:%M:%S %Z", &tm) < 15) {
		fprintf(stderr, _("stftime(3) failed\n"));
		return 1;
	}

	printf(_("IPC status from %s as of %s\n"), uts.nodename, timestr);

	return 0;
}

static int print_msg(void)
{
	int max_q, mq, rc;
	struct msginfo mi;

	printf(_("Message queues:\n"));

	max_q = msgctl(0, MSG_INFO, (struct msqid_ds *) &mi);
	if (max_q < 0) {
		perror("msgctl");
		return 1;
	}

	for (mq = 0; mq <= max_q; mq++) {
		struct msqid_ds ds;

		rc = msgctl(mq, MSG_STAT, &ds);
		if (rc < 0)
			continue;

		/* COLUMN: type of facility */
		/* COLUMN: identifier for this facility */
		/* COLUMN: key used as arg to create facility */
		/* COLUMN: The facility access modes and flags. */
		/* COLUMN: The user name/id of the owner of the entry */
		/* COLUMN: The group name/id of the owner of the entry */
		printf("m %d 0x%x --%c%c%c%c%c%c%c%c%c %d %d",
		       mq,
		       ds.msg_perm.__key,
		       ds.msg_perm.mode & 0400,	/* user read */
		       ds.msg_perm.mode & 0200,	/* user write */
		       ds.msg_perm.mode & 0200,	/* user alter */
		       ds.msg_perm.mode & 0040,	/* group read */
		       ds.msg_perm.mode & 0020,	/* group write */
		       ds.msg_perm.mode & 0020,	/* group alter */
		       ds.msg_perm.mode & 0004,	/* other read */
		       ds.msg_perm.mode & 0002,	/* other write */
		       ds.msg_perm.mode & 0002,	/* other alter */
		       ds.msg_perm.uid,
		       ds.msg_perm.gid);

		/* COLUMN: The user name of the creator of the entry */
		/* COLUMN: The group name of the creator of the entry */
		if (opt_print & ROPT_CREATOR)
			printf(" %d %d",
			       ds.msg_perm.cuid,
			       ds.msg_perm.cgid);

		/* COLUMN: number of bytes in messages currently outstanding */
		/* COLUMN: number of messages currently outstanding */
		if (opt_print & ROPT_OUTST)
			printf(" %lu %lu",
			       ds.__msg_cbytes,
			       ds.msg_qnum);

		if (opt_print & ROPT_SIZE)
			printf(" %lu", ds.msg_qbytes);

		if (opt_print & ROPT_PROC)
			printf(" %d %d",
			       ds.msg_lspid,
			       ds.msg_lrpid);

		/* FIXME: should be HHHHH:MM:SS */
		if (opt_print & ROPT_TIME)
			printf(" %lu %lu",
			       ds.msg_stime,
			       ds.msg_rtime);

		printf(" %lu\n", ds.msg_ctime);
	}

	return 0;
}

static int print_shm(void)
{
	int max_shm, shmid, rc;
	struct shminfo shi;

	printf(_("Shared memory:\n"));

	max_shm = shmctl(0, SHM_INFO, (struct shmid_ds *) &shi);
	if (max_shm < 0) {
		perror("shmctl");
		return 1;
	}

	for (shmid = 0; shmid <= max_shm; shmid++) {
		struct shmid_ds ds;

		rc = shmctl(shmid, SHM_STAT, &ds);
		if (rc < 0)
			continue;

		/* COLUMN: type of facility */
		/* COLUMN: identifier for this facility */
		/* COLUMN: key used as arg to create facility */
		/* COLUMN: The facility access modes and flags. */
		/* COLUMN: The user name/id of the owner of the entry */
		/* COLUMN: The group name/id of the owner of the entry */
		printf("m %d 0x%x --%c%c%c%c%c%c%c%c%c %d %d",
		       shmid,
		       ds.shm_perm.__key,
		       ds.shm_perm.mode & 0400,	/* user read */
		       ds.shm_perm.mode & 0200,	/* user write */
		       ds.shm_perm.mode & 0200,	/* user alter */
		       ds.shm_perm.mode & 0040,	/* group read */
		       ds.shm_perm.mode & 0020,	/* group write */
		       ds.shm_perm.mode & 0020,	/* group alter */
		       ds.shm_perm.mode & 0004,	/* other read */
		       ds.shm_perm.mode & 0002,	/* other write */
		       ds.shm_perm.mode & 0002,	/* other alter */
		       ds.shm_perm.uid,
		       ds.shm_perm.gid);

		/* COLUMN: The user name of the creator of the entry */
		/* COLUMN: The group name of the creator of the entry */
		if (opt_print & ROPT_CREATOR)
			printf(" %d %d",
			       ds.shm_perm.cuid,
			       ds.shm_perm.cgid);

		/* COLUMN: The number of processes attached to the segment */
		if (opt_print & ROPT_OUTST)
			printf(" %lu", ds.shm_nattch);

		if (opt_print & ROPT_SIZE)
			printf(" %lu", ds.shm_segsz);

		if (opt_print & ROPT_PROC)
			printf(" %d %d",
			       ds.shm_cpid,
			       ds.shm_lpid);

		/* FIXME: should be HHHHH:MM:SS */
		if (opt_print & ROPT_TIME)
			printf(" %lu %lu",
			       ds.shm_atime,
			       ds.shm_dtime);

		printf(" %lu\n", ds.shm_ctime);
	}

	return 0;
}

static int print_sem(void)
{
	int max_sem, semid, rc;
	struct seminfo sei;
	union semun arg;

	printf(_("Semaphores:\n"));

	arg.array = (unsigned short *) &sei;
	max_sem = semctl(0, 0, SEM_INFO, arg);
	if (max_sem < 0) {
		perror("semctl");
		return 1;
	}

	for (semid = 0; semid <= max_sem; semid++) {
		struct semid_ds ds;

		arg.buf = &ds;
		rc = semctl(semid, 0, SEM_STAT, arg);
		if (rc < 0)
			continue;

		/* COLUMN: type of facility */
		/* COLUMN: identifier for this facility */
		/* COLUMN: key used as arg to create facility */
		/* COLUMN: The facility access modes and flags. */
		/* COLUMN: The user name/id of the owner of the entry */
		/* COLUMN: The group name/id of the owner of the entry */
		printf("m %d 0x%x --%c%c%c%c%c%c%c%c%c %d %d",
		       semid,
		       ds.sem_perm.__key,
		       ds.sem_perm.mode & 0400,	/* user read */
		       ds.sem_perm.mode & 0200,	/* user write */
		       ds.sem_perm.mode & 0200,	/* user alter */
		       ds.sem_perm.mode & 0040,	/* group read */
		       ds.sem_perm.mode & 0020,	/* group write */
		       ds.sem_perm.mode & 0020,	/* group alter */
		       ds.sem_perm.mode & 0004,	/* other read */
		       ds.sem_perm.mode & 0002,	/* other write */
		       ds.sem_perm.mode & 0002,	/* other alter */
		       ds.sem_perm.uid,
		       ds.sem_perm.gid);

		/* COLUMN: The user name of the creator of the entry */
		/* COLUMN: The group name of the creator of the entry */
		if (opt_print & ROPT_CREATOR)
			printf(" %d %d",
			       ds.sem_perm.cuid,
			       ds.sem_perm.cgid);

		if (opt_print & ROPT_SIZE)
			printf(" %lu", ds.sem_nsems);

		/* FIXME: should be HHHHH:MM:SS */
		if (opt_print & ROPT_TIME)
			printf(" %lu",
			       ds.sem_otime);

		printf(" %lu\n", ds.sem_ctime);
	}

	return 0;
}

int main (int argc, char *argv[])
{
	pu_init();

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	if (!opt_info)
		opt_info = IOPT_ALL;
	if (!opt_print)
		opt_print = ROPT_PROC;

	if (print_header())
		return 1;

	if (opt_info & IOPT_MSG && print_msg())
		return 1;
	if (opt_info & IOPT_SHM && print_shm())
		return 1;
	if (opt_info & IOPT_SEM && print_sem())
		return 1;

	return 0;
}

