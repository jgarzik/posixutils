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
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <libpu.h>

#define PFX "tee: "

static const char doc[] =
N_("tee - read from standard input and write to standard output and files");

static struct argp_option options[] = {
	{ "append", 'a', NULL, 0,
	  N_("Append the output to the files") },
	{ "ignore-interrupts", 'i', NULL, 0,
	  N_("Ignore the SIGINT signal") },
	{ }
};

static int tee_post_walk(struct walker *w);
static int tee_init(struct walker *w, int argc, char **argv);
static int build_flist_actor(struct walker *w, const char *fn, const struct stat *lst);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, file_args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.init			= tee_init,
	.post_walk		= tee_post_walk,
	.cmdline_arg		= build_flist_actor,
};

#define ERRSTR_NOMEM PFX "out of memory\n"
#define REALLY_FREE
#define TEE_BUF_SZ 8192

static char buf[TEE_BUF_SZ];
static bool opt_append;
static bool opt_sigint;

struct tee_flist {
	const char *fn;
	int fd;
	struct tee_flist *next;
	unsigned int skip;
};

static struct tee_flist *flist_root;

static int free_flist(void)
{
	struct tee_flist *tmp;
	int err = 0;

	while (flist_root) {
		tmp = flist_root;
		flist_root = flist_root->next;

		if (close(tmp->fd) < 0) {
			perror(tmp->fn);
			err |= 1;
		}

#ifdef REALLY_FREE
		free(tmp);
#endif
	}

	flist_root = NULL;

	return err;
}

static int tee_output_bytes(const char *buf, ssize_t buflen)
{
	struct tee_flist *tmp = flist_root;
	ssize_t wrc, to_write;
	const char *s;
	int err = 0;

	while (tmp) {
		if (!tmp->skip) {
			s = buf;
			to_write = buflen;

			while (to_write > 0) {
				wrc = write(tmp->fd, s, to_write);
				if (wrc < 1) {
					perror(tmp->fn);
					tmp->skip = 1;
					err = 1;
					break;
				}

				s += wrc;
				to_write -= wrc;
			}
		}

		tmp = tmp->next;
	}

	return err;
}

static int tee_output(void)
{
	int rc, err = 0;
	fd_set rd_set, err_set;
	ssize_t bread;

	rc = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	if (rc < 0) {
		perror(STDIN_NAME);
		err = 1;
		goto out;
	}

	FD_ZERO(&rd_set);
	FD_ZERO(&err_set);
	while (1) {
		FD_SET(STDIN_FILENO, &rd_set);
		FD_SET(STDIN_FILENO, &err_set);

		rc = select(STDIN_FILENO + 1, &rd_set, NULL, &err_set, NULL);
		if (rc < 1) {
			perror(STDIN_NAME);
			break;
		}

		bread = read(STDIN_FILENO, buf, sizeof(buf));
		if (bread == 0)
			break;
		if (bread < 0) {
			perror(STDIN_NAME);
			rc = 1;
			break;
		}

		err |= tee_output_bytes(buf, bread);
	}

out:
	err |= free_flist();

	return err;
}

static int build_flist_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	struct tee_flist *new, *tmp;
	int open_flags;
	int fd;

	if (opt_append)
		open_flags = O_WRONLY | O_CREAT | O_APPEND;
	else
		open_flags = O_WRONLY | O_CREAT | O_TRUNC;

	fd = open(fn, open_flags, 0666);
	if (fd < 0) {
		perror(fn);
		return 1;
	}

	new = xmalloc(sizeof(*new));
	new->fn = fn;
	new->fd = fd;
	new->next = NULL;

	tmp = flist_root;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new;

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('a', append)
	PU_OPT_SET('i', sigint)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int tee_init(struct walker *w, int argc, char **argv)
{
	struct tee_flist *new;

	pu_init();

	new = xmalloc(sizeof(*new));
	new->fn = STDOUT_NAME;
	new->fd = STDOUT_FILENO;
	new->next = NULL;
	flist_root = new;

	return 0;
}

static int tee_post_walk(struct walker *w)
{
	if (opt_sigint && signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, PFX "cannot ignore SIGINT\n");
		return 1;
	}

	return tee_output();
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
