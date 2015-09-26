
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
#define _GNU_SOURCE		/* for O_DIRECTORY */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <alloca.h>
#include <pwd.h>
#include <grp.h>
#include <libpu.h>

using namespace std;

static const char doc[] =
N_("chown - change file owner and group");

static const char args_doc[] = N_("owner[:group] file ...");

static struct argp_option options[] = {
	{ NULL, 'h', NULL, 0,
	  N_("Set uid/gid of symlink, if file is a symlink.") },
	{ NULL, 'H', NULL, 0,
	  N_("Recurse into symlinked dirs on the command line.") },
	{ NULL, 'L', NULL, 0,
	  N_("Recurse into symlinked dirs on the command line or found during traversal.") },
	{ NULL, 'P', NULL, 0,
	  N_("Do not follow symlinks.") },
	{ NULL, 'R', NULL, 0,
	  N_("Recursively change file user and group IDs.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static int chown_init(struct walker *w, int argc, char **argv);
static int chown_pre_walk(struct walker *w);
static int chown_arg(struct walker *w, const char *fn, const struct stat *lst);
static int chown_dirent(struct walker *w, const char *dirn, const char *basen,
			const struct stat *lst);

static bool opt_set_link_attr;
static bool opt_set_link_attr_recur;
static bool opt_follow_link_cmdline;
static bool opt_follow_link;
static bool opt_recurse;
static bool opt_chgrp;
static uid_t opt_uid = (uid_t) -1;
static gid_t opt_gid = (gid_t) -1;

static const struct argp argp = { options, parse_opt, args_doc, doc };
static struct walker walker;


static int set_group(const char *group)
{
	unsigned int u;

	struct group *gr = getgrnam(group);
	if (gr == NULL) {
		if (sscanf(group, "%u", &u) != 1) {
			fprintf(stderr, _("invalid group '%s'\n"), group);
			return ARGP_ERR_UNKNOWN;
		}
		opt_gid = u;
	} else
		opt_gid = gr->gr_gid;

	return 0;
}

static int set_owner_group(const char *owner_group)
{
	char *owner, *group;
	int rc, have_group = 0;
	size_t len;
	struct passwd *pw;
	unsigned int u;

	len = strlen(owner_group);
	owner = (char *) alloca(len + 1);
	group = (char *) alloca(len + 1);

	rc = sscanf(owner_group, "%s:%s", owner, group);
	if (rc == 2)
		have_group = 1;
	else if (rc != 1) {
		fprintf(stderr, _("owner[:group] not provided\n"));
		return ARGP_ERR_UNKNOWN;
	}

	pw = getpwnam(owner);
	if (pw == NULL) {
		if (sscanf(owner, "%u", &u) != 1) {
			fprintf(stderr, _("invalid owner '%s'\n"), owner_group);
			return ARGP_ERR_UNKNOWN;
		}
		opt_uid = u;
	} else
		opt_uid = pw->pw_uid;

	if (have_group)
		rc = set_group(group);
	else
		rc = 0;

	return rc;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	PU_OPT_SET('h', set_link_attr)
	PU_OPT_SET('R', recurse)
	PU_OPT_ARG

	case 'H':
		opt_set_link_attr_recur = false;
		opt_follow_link_cmdline = true;
		opt_follow_link = false;
		break;
	case 'L':
		opt_set_link_attr_recur = false;
		opt_follow_link_cmdline = true;
		opt_follow_link = true;
		break;
	case 'P':
		opt_set_link_attr_recur = true;
		opt_follow_link_cmdline = false;
		opt_follow_link = false;
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 2)		/* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static void perror_fn(const char *dir, const char *base)
{
	int saved_errno = errno;
	char *fn = strpathcat(dir, base);
	errno = saved_errno;
	perror(fn);
	free(fn);
}

static int need_chown (const struct stat *st)
{
	if ((opt_uid != (uid_t)-1) && (st->st_uid != opt_uid))
		return 1;
	if ((opt_gid != (gid_t)-1) && (st->st_gid != opt_gid))
		return 1;
	return 0;
}

static int chown_dirent(struct walker *w, const char *dirn, const char *basen,
			const struct stat *lst)
{
	int is_link = 0, is_dir = 0;

	if (S_ISLNK(lst->st_mode))
		is_link = 1;
	else if (S_ISDIR(lst->st_mode))
		is_dir = 1;

	if (!is_link) {
		/* race between lstat(2) and here */
		if (need_chown(lst) && chown(basen, opt_uid, opt_gid) < 0) {
			perror_fn(dirn, basen);
			w->exit_status = 1;
			return RC_OK;
		}

		if ((!is_dir) || (!opt_recurse))
			return RC_OK;
	}

	else {
		struct stat st;

		if (!opt_follow_link)
			return RC_OK;

		/* race between lstat(2) and here */
		if (stat(basen, &st) < 0) {
			perror_fn(dirn, basen);
			w->exit_status = 1;
			return RC_OK;
		}

		if (!S_ISDIR(st.st_mode))
			return RC_OK;
	}

	return RC_RECURSE;
}

static int chown_arg(struct walker *w, const char *fn, const struct stat *lst)
{
	int is_link = 0, is_dir = 0;

	if (S_ISLNK(lst->st_mode))
		is_link = 1;
	else if (S_ISDIR(lst->st_mode))
		is_dir = 1;

	if (!is_link) {
		/* race between lstat(2) and here */
		if (need_chown(lst) && chown(fn, opt_uid, opt_gid) < 0) {
			perror(fn);
			w->exit_status = 1;
			return RC_OK;
		}

		if ((!is_dir) || (!opt_recurse))
			return RC_OK;
	}

	if (is_link) {
		struct stat st;

		if (opt_set_link_attr) {
			/* race between lstat(2) and here */
			if (lchown(fn, opt_uid, opt_gid) < 0) {
				perror(fn);
				w->exit_status = 1;
			}
			return RC_OK;
		}

		if (!opt_follow_link_cmdline)
			return RC_OK;

		/* race between lstat(2) and here */
		if (stat(fn, &st) < 0) {
			perror(fn);
			w->exit_status = 1;
			return RC_OK;
		}

		if (!S_ISDIR(st.st_mode))
			return RC_OK;
	}

	return RC_RECURSE;
}

static int chown_init(struct walker *w, int argc, char **argv)
{
	pu_init();

	struct pathelem *pe = path_split(argv[0]);
	if (!strcmp(pe->basen, "chgrp"))
		opt_chgrp = true;
	path_free(pe);

	return 0;
}

static int chown_pre_walk(struct walker *w)
{
	string arg = w->arglist.front();
	w->arglist.erase(w->arglist.begin());

	if (opt_chgrp)
		set_group(arg.c_str());
	else
		set_owner_group(arg.c_str());

	if (!opt_recurse) {
		opt_set_link_attr_recur = false;
		opt_follow_link_cmdline = false;
		opt_follow_link = false;
	} else {
		w->flags |= WF_RECURSE;
		if (opt_set_link_attr_recur)
			opt_set_link_attr = true;
	}

	if (opt_follow_link)
		w->flags |= WF_FOLLOW_LINK;
	if (opt_follow_link_cmdline)
		w->flags |= WF_FOLLOW_LINK_CMDLINE;

	return 0;
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.flags			= WF_STAT;
	walker.init			= chown_init;
	walker.pre_walk			= chown_pre_walk;
	walker.cmdline_arg		= chown_arg;
	walker.dirent			= chown_dirent;
	return walk(&walker, argc, argv);
}
