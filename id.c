
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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <argp.h>
#include <libpu.h>

static const char doc[] =
N_("id - return user identity");

static const char args_doc[] = N_("[user]");
static struct argp_option options[] = {
	{ NULL, 'g', NULL, 0,
	  N_("Output only the effective group ID") },
	{ NULL, 'G', NULL, 0,
	  N_("Output all different group IDs") },
	{ NULL, 'n', NULL, 0,
	  N_("Output the name instead of the numeric ID") },
	{ NULL, 'r', NULL, 0,
	  N_("Output the real ID instead of the effective ID") },
	{ NULL, 'u', NULL, 0,
	  N_("Output only the effective user ID") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static bool opt_name;
static bool opt_real_id;
static enum opt_mode {
	ID_DEF,
	ID_GRP_ALL,
	ID_EGID,
	ID_EUID,
} opt_mode;
static char *opt_user;

static const struct argp argp = { options, parse_opt, args_doc, doc };

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'n':
		opt_name = true;
		break;
	case 'r':
		opt_real_id = true;
		break;
	case 'g':
		opt_mode = ID_EGID;
		break;
	case 'G':
		opt_mode = ID_GRP_ALL;
		break;
	case 'u':
		opt_mode = ID_EUID;
		break;

	case ARGP_KEY_ARG:
		if (state->arg_num > 0)		/* too many args */
			argp_usage (state);
		opt_user = arg;
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int user_in_group (const struct group *gr)
{
	unsigned int i = 0;
	char *s;

	if (!opt_user)
		return 0;

	while ((s = gr->gr_mem[i++]) != NULL)
		if (!strcmp(s, opt_user))
			return 1;

	return 0;
}

struct grpent;
struct grpent {
	gid_t		gid;
	struct grpent	*next;
	char		name[0];
};

static void add_grpent(struct grpent **head, struct grpent **tail_io,
		      const struct group *gr)
{
	struct grpent *ent, *tail = *tail_io;

	ent = xmalloc(sizeof(struct grpent) + strlen(gr->gr_name) + 1);
	ent->gid = gr->gr_gid;
	ent->next = NULL;
	strcpy(ent->name, gr->gr_name);

	if (!*head)
		*head = ent;
	if (tail)
		tail->next = ent;
	*tail_io = ent;
}

static struct grpent *match_groups(gid_t gid, gid_t egid)
{
	struct group *gr;
	struct grpent *rc = NULL, *head = NULL, *tail = NULL;

	setgrent();

	while (1) {
		errno = 0;
		gr = getgrent();
		if (!gr) {
			if (errno) {
				perror("getgrent");
				goto out;
			}
			break;
		}

		if (gr->gr_gid == gid || gr->gr_gid == egid ||
		    user_in_group(gr)) {
			add_grpent(&head, &tail, gr);
		}
	}

	rc = head;

out:
	endgrent();
	return rc;
}

static void pr_uid(uid_t uid, bool space, bool newline)
{
	unsigned long long uid_ull;

	if (opt_name) {
		struct passwd *pw;

		errno = 0;
		pw = getpwuid(uid);
		if (!pw && errno)
			perror("warning(getpwuid)");
		if (pw) {
			printf("%s%s%s\n",
			       space ? " " : "",
			       pw->pw_name,
			       newline ? "\n" : "");
			return;
		}
	}

	uid_ull = uid;
	printf("%s%Lu%s\n",
	       space ? " " : "",
	       uid_ull,
	       newline ? "\n" : "");
}

static void pr_gid(gid_t gid, bool space, bool newline)
{
	unsigned long long gid_ull;

	if (opt_name) {
		struct group *gr;

		errno = 0;
		gr = getgrgid(gid);
		if (!gr && errno)
			perror("warning(getgrgid)");
		if (gr) {
			printf("%s%s%s\n",
			       space ? " " : "",
			       gr->gr_name,
			       newline ? "\n" : "");
			return;
		}
	}

	gid_ull = gid;
	printf("%s%Lu%s\n",
	       space ? " " : "",
	       gid_ull,
	       newline ? "\n" : "");
}

static int id_grp_all(struct grpent *grp, gid_t gid, gid_t egid)
{
	struct grpent *tmp;
	char *s, s1[64];

	tmp = grp;
	while (tmp) {
		if (opt_name)
			s = tmp->name;
		else {
			snprintf(s1, sizeof(s1), "%Lu",
				 (unsigned long long) tmp->gid);
			s = s1;
		}

		printf("%s%s%s",
		       (tmp == grp) ? "" : " ",
		       s,
		       (tmp->next) ? "\n" : "");

		tmp = tmp->next;
	}

	return 0;
}

static int id_def(uid_t uid, uid_t euid, gid_t gid, gid_t egid,
		  struct grpent *grp)
{
	struct passwd *pw;
	struct group *gr;

	printf("uid=%Lu%s%s%s",
	       (unsigned long long) uid,
	       opt_user ? "(" : "",
	       opt_user ? opt_user : "",
	       opt_user ? ")" : "");

	if (uid != euid) {
		pw = getpwuid(euid);
		printf(" euid=%Lu%s%s%s",
		       (unsigned long long) euid,
		       pw ? "(" : "",
		       pw ? pw->pw_name : "",
		       pw ? ")" : "");
	}

	gr = getgrgid(gid);
	printf(" gid=%Lu%s%s%s",
	       (unsigned long long) gid,
	       gr ? "(" : "",
	       gr ? gr->gr_name : "",
	       gr ? ")" : "");

	if (gid != egid) {
		gr = getgrgid(egid);
		printf(" egid=%Lu%s%s%s",
		       (unsigned long long) egid,
		       gr ? "(" : "",
		       gr ? gr->gr_name : "",
		       gr ? ")" : "");
	}

	if (grp) {
		struct grpent *tmp = grp;
		printf(" groups=");

		while (tmp) {
			printf("%s%Lu(%s)",
			       (tmp == grp) ? "" : ",",
			       (unsigned long long) tmp->gid,
			       tmp->name);
			tmp = tmp->next;
		}
	}

	printf("\n");

	return 0;
}

static int do_id(void)
{
	uid_t uid, euid;
	gid_t gid, egid;
	struct passwd *pw;
	struct grpent *grp;

	if (opt_user) {
		errno = 0;
		pw = getpwnam(opt_user);
		if (!pw) {
			if (errno)
				perror(opt_user);
			else
				fprintf(stderr, "user '%s' not found\n",
					opt_user);
			return 1;
		}

		uid = euid = pw->pw_uid;
		gid = egid = pw->pw_gid;
	} else {
		uid = getuid();
		euid = geteuid();
		gid = getgid();
		egid = getegid();

		if (opt_real_id) {
			euid = uid;
			egid = gid;
		}

		if (opt_mode == ID_GRP_ALL || opt_mode == ID_DEF) {
			pw = getpwuid(uid);
			if (pw)
				opt_user = xstrdup(pw->pw_name);
		}
	}

	switch (opt_mode) {
	case ID_EUID:	pr_uid(euid, false, true); return 0;
	case ID_EGID:	pr_gid(egid, false, true); return 0;
	default:	break;
	}

	grp = match_groups(gid, egid);
	if (!grp)
		return 1;

	switch (opt_mode) {
	case ID_GRP_ALL:	return id_grp_all(grp, gid, egid);
	case ID_DEF:		return id_def(uid, euid, gid, egid, grp);
	default:		return 1;
	}
}

int main (int argc, char *argv[])
{
	error_t rc;

	pu_init();

	rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (rc) {
		fprintf(stderr, "argp_parse failed: %s\n", strerror(rc));
		return 1;
	}

	return do_id();
}

