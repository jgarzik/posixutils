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
#define _GNU_SOURCE 1			/* for O_DIRECTORY */
#define _DARWIN_C_SOURCE 1		/* for O_DIRECTORY */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <argp.h>
#include <dirent.h>
#include <ctype.h>
#include <locale.h>
#include <libpu.h>

const char file_args_doc[] = N_("file...");

struct argp_option no_options[] = { { } };

#define COPY_BUF_SZ	8192

static char buf[COPY_BUF_SZ];

ssize_t copy_fd(const char *dest_fn, int dest_fd,
		const char *src_fn, int src_fd)
{
	char *s;
	ssize_t rc, wrc;
	size_t bytes = 0;

	while (1) {
		rc = read(src_fd, buf, sizeof(buf));
		if (rc < 0) {
			perror(src_fn);
			return -1;
		}
		if (rc == 0)
			break;

		s = buf;
		while (rc > 0) {
			wrc = write(dest_fd, s, rc);
			if (wrc < 0) {
				perror(dest_fn);
				return -1;
			}

			rc -= wrc;
			s += wrc;
			bytes += wrc;
		}
	}

	return bytes;
}

int write_fd(int fd, const void *buf_, size_t count, const char *fn)
{
	const char *buf = (const char *) buf_;

	while (count > 0) {
		ssize_t wrc = write(fd, buf, count);
		if (wrc < 0) {
			perror(fn);
			return 1;
		}

		buf += wrc;
		count -= wrc;
	}

	return 0;
}

int write_buf(const void *buf, size_t len)
{
	return write_fd(STDOUT_FILENO, buf, len, STDOUT_NAME);
}

void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

error_t noopts_parse_opt (int key, char *arg, struct argp_state *state)
{
	return ARGP_ERR_UNKNOWN;
}

int ask_question(const char *prefix, const char *msg,
			const char *fn)
{
	char s[32], *src;

	fprintf(stderr, msg, prefix, fn);

	src = fgets_unlocked(s, sizeof(s), stdin);
	if ((!src) || (toupper(s[0]) != 'Y'))
		return 0;

	return 1;
}

int have_dots(const char *fn)
{
	return (!strcmp(fn, ".")) || (!strcmp(fn, ".."));
}

void pu_init(void)
{
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	__fsetlocking(stdin, FSETLOCKING_BYCALLER);
	__fsetlocking(stdout, FSETLOCKING_BYCALLER);
	__fsetlocking(stderr, FSETLOCKING_BYCALLER);

	argp_program_version = PACKAGE_VERSION;
	argp_program_bug_address = PACKAGE_BUGREPORT;
	argp_err_exit_status = EXIT_FAILURE;
}

int ro_file_open(FILE **f_o, const char *fn)
{
	FILE *f = fopen(fn, "r");
	if (!f) {
		perror(fn);
		return 1;
	}

#ifdef POSIX_FADV_SEQUENTIAL
	/* Do we care about the return value of posix_fadvise(2) ? */
	posix_fadvise(fileno(f), 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

	__fsetlocking(f, FSETLOCKING_BYCALLER);
	*f_o = f;
	return 0;
}

int iterate_directory(int old_dirfd, const char *dirfn, const char *basen,
		      int opt_force, idir_actor_t actor)
{
	DIR *dir;
	char *fn;
	int dirfd, rc = 0;
	struct dirent *de;

	fn = (char *) xmalloc(strlen(dirfn) + strlen(basen) + 2);
	sprintf(fn, "%s/%s", dirfn, basen);

	dirfd = open(basen, O_DIRECTORY);
	if (dirfd < 0) {
		if (!opt_force) {
			perror(fn);
			rc = 1;
			goto out;
		}
	}

	if (fchdir(dirfd) < 0) {
		perror(fn);
		rc = 1;
		goto out_fd;
	}

	dir = opendir(".");
	if (!dir) {
		perror(fn);
		rc = 1;
		goto out_chdir;
	}

	while ((de = readdir(dir)) != NULL)
		rc |= actor(dirfd, fn, de->d_name);

	if (closedir(dir) < 0) {
		perror(fn);
		rc = 1;
	}

out_chdir:
	if (fchdir(old_dirfd) < 0) {
		perror(dirfn);
		rc = 1;
	}
out_fd:
	if (close(dirfd) < 0) {
		perror(fn);
		rc = 1;
	}
out:
	free(fn);
	return rc;
}

int is_portable_char(int ch)
{
	if ((ch == '-') || (ch == '.'))
		return 1;
	if ((ch >= '0') && (ch <= '9'))
		return 1;
	if ((ch >= 'A') && (ch <= 'Z'))
		return 1;
	if (ch == '_')
		return 1;
	if ((ch >= 'a') && (ch <= 'z'))
		return 1;
	return 0;
}

int map_lookup(const struct strmap *map, const char *key)
{
	while (map->name) {
		if (!strcmp(map->name, key))
			return map->val;
		map++;
	}

	return -1;
}

