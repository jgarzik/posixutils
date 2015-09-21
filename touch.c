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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <libpu.h>

#define PFX "touch: "

static const char doc[] =
N_("touch - change file timestamps");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ NULL, 'a', NULL, 0,
	  N_("Change the access time of file") },
	{ "no-create", 'c', NULL, 0,
	  N_("Do not create a specified file if it does not exist") },
	{ NULL, 'm', NULL, 0,
	  N_("Change the modification time of file") },
	{ "reference", 'r', "FILE", 0,
	  N_("Use the corresponding time of FILE instead of the current time") },
	{ NULL, 't', "STAMP", 0,
	  N_("Use the specified time STAMP instead of the current time") },
	{ }
};

static int touch_init(struct walker *w, int argc, char **argv);
static int touch_pre_walk(struct walker *w);
static int touch_fn_actor(struct walker *w, const char *fn, const struct stat *lst);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker;

static bool opt_default = true;
static bool opt_atime;
static bool opt_creat_ok = true;
static bool opt_mtime;
static bool opt_ref_file;
static time_t touch_atime, touch_mtime, touch_time;
static struct stat touch_st;


static int touch_fn_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	struct utimbuf ut;
	struct stat st;
	int fd, second_time = 0;

	ut.actime = touch_atime;
	ut.modtime = touch_mtime;

	if (!opt_atime || !opt_mtime) {
		if (stat(fn, &st) < 0)
			goto err_out;

		if (!opt_atime)
			ut.actime = st.st_atime;
		if (!opt_mtime)
			ut.modtime = st.st_mtime;
	}

again_butthead:
	if (utime(fn, &ut) == 0)
		return 0;
	if (errno != ENOENT)
		goto err_out;
	if (!opt_creat_ok || second_time)
		return 1;

	fd = creat(fn, 0666);
	if (fd < 0)
		goto err_out;

	second_time = 1;
	goto again_butthead;

err_out:
	perror(fn);
	return 1;
}

static void parse_user_time(char *ut)
{
	struct tm tm;
	char *suff;
	int rc = 0, req_tokens = 99;

	memset(&tm, 0, sizeof(tm));

	suff = strchr(ut, '.');
	if (suff) {
		*suff = 0;
		suff++;
	}

	touch_time = time(NULL);
	if (strlen(ut) == 8) {
		localtime_r(&touch_time, &tm);
		req_tokens = 4;
		rc = sscanf(ut, "%02d%02d%02d%02d",
			    &tm.tm_mon,
			    &tm.tm_mday,
			    &tm.tm_hour,
			    &tm.tm_min);
	}
	else if (strlen(ut) == 10) {
		req_tokens = 5;
		rc = sscanf(ut, "%02d%02d%02d%02d%02d",
			    &tm.tm_year,
			    &tm.tm_mon,
			    &tm.tm_mday,
			    &tm.tm_hour,
			    &tm.tm_min);
		if (rc == 5) {
			if (tm.tm_year >= 69)
				tm.tm_year += 1900;
			else
				tm.tm_year += 2000;
		}
	}
	else if (strlen(ut) == 12) {
		req_tokens = 5;
		rc = sscanf(ut, "%04d%02d%02d%02d%02d",
			    &tm.tm_year,
			    &tm.tm_mon,
			    &tm.tm_mday,
			    &tm.tm_hour,
			    &tm.tm_min);
	}

	if (rc != req_tokens) {
		fprintf(stderr, PFX "invalid time spec\n");
		exit(1);
	}

	if (suff) {
		req_tokens = 1;
		rc = sscanf(suff, "%02d", &tm.tm_sec);
	}

	if (rc != req_tokens) {
		fprintf(stderr, PFX "invalid time spec\n");
		exit(1);
	}

	touch_time = mktime(&tm);
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 'a':
		opt_default = false;
		opt_atime = true;
		break;
	case 'c':
		opt_creat_ok = false;
		break;
	case 'm':
		opt_default = false;
		opt_mtime = true;
		break;
	case 'r':
		if (stat(arg, &touch_st) < 0) {
			perror(arg);
			exit(1);
		}
		opt_ref_file = true;
		break;
	case 't':
		opt_ref_file = false;
		parse_user_time(optarg);
		break;

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int touch_init(struct walker *w, int argc, char **argv)
{
	pu_init();
	touch_time = time(NULL);
	return 0;
}

static int touch_pre_walk(struct walker *w)
{
	if (opt_default)
		opt_mtime = opt_atime = true;

	if (opt_ref_file) {
		touch_atime = touch_st.st_atime;
		touch_mtime = touch_st.st_mtime;
	} else {
		touch_atime = touch_time;
		touch_mtime = touch_time;
	}

	return 0;
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.init			= touch_init;
	walker.pre_walk			= touch_pre_walk;
	walker.cmdline_arg		= touch_fn_actor;
	return walk(&walker, argc, argv);
}
