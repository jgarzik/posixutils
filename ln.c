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

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <libgen.h>
#include <libpu.h>

#define PFX "ln: "

static const char doc[] =
N_("ln - make links between files");

static const char args_doc[] = N_("file... TARGET");

static struct argp_option options[] = {
	{ "force", 'f', NULL, 0,
	  N_("Force existing dest. pathnames to be removed to allow the link") },
	{ "symbolic", 's', NULL, 0,
	  N_("Create symbolic links instead of hard links") },
	{ }
};

static int ln_fn_actor(struct walker *w, const char *fn, const struct stat *lst);
static int ln_pre_walk(struct walker *w);
static int ln_post_walk(struct walker *w);
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.pre_walk		= ln_pre_walk,
	.post_walk		= ln_post_walk,
	.cmdline_arg		= ln_fn_actor,
};

static char pathbuf[PATH_MAX + 2];
static char basenamebuf[PATH_MAX + 2];
static char linkbuf[PATH_MAX + 1];
static const char *target;
static bool opt_force;
static bool opt_symlink;
static bool have_2arg_form = true;


static int do_link(struct walker *w, const char *link_target, const char *link_name)
{
	int rc;
	ssize_t linklen;

	if (access(link_name, F_OK) == 0) {
		if (!opt_force) {
			fprintf(stderr, PFX "%s: exists, skipping\n",
				link_name);
			return 1;
		}

		if (unlink(link_name) < 0)
			goto perror_linkname;
	}

	if (opt_symlink) {
		rc = symlink(link_target, link_name);
		if (rc < 0)
			goto perror_linkname;

		return 0;
	}

	linklen = readlink(link_target, linkbuf, sizeof(linkbuf));
	if (linklen > 0) {
		if (linklen > PATH_MAX) {
			fprintf(stderr, PFX "link target too long, skipping\n");
			w->exit_status = 1;
			return 0;
		}
		linkbuf[linklen] = 0;
		link_target = linkbuf;
	}

	rc = link(link_target, link_name);
	if (rc < 0)
		goto perror_linkname;

	return 0;

perror_linkname:
	perror(link_name);
	return 1;
}

static int ln_fn_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	int slen;
	char *base;

	if (have_2arg_form)
		return 0;

	strncpy(basenamebuf, fn, sizeof(basenamebuf) - 1);
	basenamebuf[sizeof(basenamebuf) - 1] = 0;
	base = basename(basenamebuf);

	slen = snprintf(pathbuf, sizeof(pathbuf), "%s/%s", target, base);
	if (slen > PATH_MAX) {
		fprintf(stderr, PFX "link target too long, skipping\n");
		return 1;
	}

	w->exit_status = do_link(w, fn, pathbuf);
	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('f', force)
	PU_OPT_SET('s', symlink)
	PU_OPT_ARG

	case ARGP_KEY_END:
		if (state->arg_num < 2)		/* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	PU_OPT_DEFAULT
	PU_OPT_END
}

static int ln_pre_walk(struct walker *w)
{
	struct stat st;
	int rc;

	if (w->strlist.len < 2) {
		fprintf(stderr, _("ln: missing link source/target\n"));
		return 1;
	}

	target = slist_pop(&w->strlist);
	rc = stat(target, &st);
	if ((rc == 0) && (S_ISDIR(st.st_mode)))
		have_2arg_form = false;

	return 0;
}

static int ln_post_walk(struct walker *w)
{
	if (have_2arg_form) {
		if (w->strlist.len != 1) {
			fprintf(stderr, _("ln: too many arguments, when "
					"target is not directory\n"));
			return 1;
		}

		return do_link(w, slist_ref(&w->strlist, 0), target);
	}

	return w->exit_status;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}
