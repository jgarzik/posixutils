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

#define __STDC_FORMAT_MACROS 1

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#ifdef __linux__
#include <mntent.h>
#include <sys/vfs.h>
#elif __APPLE__
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif
#include <paths.h>
#include <libpu.h>

using namespace std;

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

static struct walker walker;

class FSListEnt {
public:
	string			devname;
	string			dir;
	dev_t			dev;
	bool			masked;
};

static uintmax_t opt_block_size = 512;
static bool opt_portable;
static bool opt_total_alloc;
static bool fslist_masked;

static vector<FSListEnt> fslist;


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

#ifdef __linux__
static void push_mntent(struct mntent *me)
{
	struct stat st;

	FSListEnt fl;
	fl.devname.assign(me->mnt_fsname);
	fl.dir.assign(me->mnt_dir);

	if (stat(fl.devname.c_str(), &st) == 0)
		fl.dev = st.st_rdev;
	else if (stat(fl.dir.c_str(), &st) == 0)
		fl.dev = st.st_dev;
	else
		fl.dev = (dev_t) -1;

	fslist.push_back(fl);
}
#endif

#ifdef __APPLE__
static void push_mntent(const struct statfs *sfs)
{
	struct stat st;

	FSListEnt fl;
	fl.devname.assign(sfs->f_mntfromname);
	fl.dir.assign(sfs->f_mntonname);

	if (stat(fl.devname.c_str(), &st) == 0)
		fl.dev = st.st_rdev;
	else if (stat(fl.dir.c_str(), &st) == 0)
		fl.dev = st.st_dev;
	else
		fl.dev = (dev_t) -1;

	fslist.push_back(fl);
}
#endif

static int read_mntlist(void)
{
#ifdef __linux__
	struct mntent *me;

	FILE *f = setmntent(_PATH_MOUNTED, "r");
	if (!f) {
		perror(_PATH_MOUNTED);
		return 1;
	}

	while ((me = getmntent(f)) != NULL)
		push_mntent(me);

	endmntent(f);
#elif __APPLE__
	struct statfs* mounts = NULL;
	int n_mnt = getmntinfo(&mounts, MNT_WAIT);
	if (n_mnt < 0) {
		perror("getmntinfo");
		return 1;
	}

	for (int i = 0; i < n_mnt; i++) {
		push_mntent(&mounts[i]);
	}
#endif

	return 0;
}

static int df_arg(struct walker *w, const char *fn, const struct stat *st)
{
	fslist_masked = true;

	for (unsigned int i = 0; i < fslist.size(); i++) {
		FSListEnt& tmp = fslist[i];

		if (tmp.dev == st->st_dev)
			tmp.masked = true;
	}

	return 0;
}

static void df_mask_all(void)
{
	for (unsigned int i = 0; i < fslist.size(); i++)
		fslist[i].masked = true;
}

static int df_output(FSListEnt& fl)
{
	struct statfs sf;
	uintmax_t blksz, total, used, avail, free, pct;
	const char *fmt;

	if (statfs(fl.dir.c_str(), &sf) < 0) {
		perror(fl.dir.c_str());
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
	       fl.devname.c_str(),
	       total,
	       used,
	       avail,
	       pct,
	       fl.dir.c_str());

	return 0;
}

static int df_output_fslist(void)
{
	int rc = 0;

	if (opt_portable)
		printf(_("Filesystem         %4ju-blocks      Used Available Capacity Mounted on\n"),
	       		opt_block_size);
	else
		printf(_("Filesystem         %4ju-blocks      Used Available Use%% Mounted on\n"),
	       		opt_block_size);

	for (unsigned int i = 0; i < fslist.size(); i++) {
		FSListEnt& tmp = fslist[i];

		if (tmp.masked)
			rc |= df_output(tmp);
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
	walker.argp			= &argp;
	walker.flags			= WF_STAT;
	walker.pre_walk			= df_pre_walk;
	walker.post_walk		= df_post_walk;
	walker.cmdline_arg		= df_arg;
	return walk(&walker, argc, argv);
}

