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

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <paths.h>
#include <libpu.h>


static const char doc[] =
N_("df - report filesystem disk space usage");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ NULL, 'k', NULL, 0,
	  N_("Use 1024-byte units, instead of the default 512-byte units") },
	{ "portability", 'P', NULL, 0,
	  N_("Use POSIX-defined output format") },
	{ NULL, 't', NULL, 0,
	  N_("Include total allocated-space figures in the output.") },
	{ }
};

static int df_post_walk(struct walker *w);
static int df_pre_walk(struct walker *w);
static int df_arg(struct walker *w, const char *fn, const struct stat *lst);

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_STAT,
	.pre_walk		= df_pre_walk,
	.post_walk		= df_post_walk,
	.cmdline_arg		= df_arg,
};

struct fslist {
	char			*devname;
	char			*dir;
	dev_t			dev;
	bool			masked;
	struct fslist		*next;
};

static uintmax_t opt_block_size = 512;
static bool opt_portable;
static bool opt_total_alloc;
static bool fslist_masked;

static struct fslist *fslist, *fslist_last;


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 'k':
		opt_block_size = 1024;
		break;

	PU_OPT_SET('P', portable)
	PU_OPT_SET('t', total_alloc)
	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static void push_mntent(struct mntent *me)
{
	struct stat st;

	struct fslist *fl = xcalloc(1, sizeof(struct fslist));
	fl->devname = xstrdup(me->mnt_fsname);
	fl->dir = xstrdup(me->mnt_dir);

	if (stat(fl->devname, &st) == 0)
		fl->dev = st.st_rdev;
	else if (stat(fl->dir, &st) == 0)
		fl->dev = st.st_dev;
	else
		fl->dev = (dev_t) -1;

	if (!fslist) {
		fslist = fl;
		fslist_last = fl;
	} else {
		fslist_last->next = fl;
		fslist_last = fl;
	}
}

static int read_mntlist(void)
{
	struct mntent *me;

	FILE *f = setmntent(_PATH_MOUNTED, "r");
	if (!f) {
		perror(_PATH_MOUNTED);
		return 1;
	}

	while ((me = getmntent(f)) != NULL)
		push_mntent(me);
	
	endmntent(f);

	return 0;
}

static int df_arg(struct walker *w, const char *fn, const struct stat *st)
{
	struct fslist *tmp = fslist;

	fslist_masked = true;

	while (tmp) {
		if (tmp->dev == st->st_dev)
			tmp->masked = true;
		tmp = tmp->next;
	}

	return 0;
}

static void df_mask_all(void)
{
	struct fslist *tmp = fslist;
	while (tmp) {
		tmp->masked = true;
		tmp = tmp->next;
	}
}

static int df_output(struct fslist *fl)
{
	struct statfs sf;
	uintmax_t blksz, total, used, avail, free, pct;
	const char *fmt;

	if (statfs(fl->dir, &sf) < 0) {
		perror(fl->dir);
		return 1;
	}

	blksz = sf.f_bsize;

	total = sf.f_blocks;
	total *= blksz;
	total /= opt_block_size;

	avail = sf.f_bavail;
	avail *= blksz;
	avail /= opt_block_size;

	free = sf.f_bfree;
	free *= blksz;
	free /= opt_block_size;

	used = total - free;

	if (total == 0)
		return 0;

	pct = ((total - avail) * 100) / total;

	if (opt_portable)
		fmt = _("%-20s %9" PRIuMAX " %9" PRIuMAX " %9" PRIuMAX " %7" PRIuMAX "%% %s\n");
	else
		fmt = _("%-20s %9" PRIuMAX " %9" PRIuMAX " %9" PRIuMAX " %3" PRIuMAX "%% %s\n");

	printf(fmt,
	       fl->devname,
	       total,
	       used,
	       avail,
	       pct,
	       fl->dir);

	return 0;
}

static int df_output_fslist(void)
{
	struct fslist *tmp = fslist;
	int rc = 0;

	if (opt_portable)
		printf(_("Filesystem         %4llu-blocks      Used Available Capacity Mounted on\n"),
	       		opt_block_size);
	else
		printf(_("Filesystem         %4llu-blocks      Used Available Use%% Mounted on\n"),
	       		opt_block_size);

	while (tmp) {
		if (tmp->masked)
			rc |= df_output(tmp);
		tmp = tmp->next;
	}

	return rc;
}

static int df_pre_walk(struct walker *w)
{
	return read_mntlist();
}

static int df_post_walk(struct walker *w)
{
	if (!fslist_masked)
		df_mask_all();

	return df_output_fslist();
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
