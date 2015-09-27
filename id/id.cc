
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
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <argp.h>
#include <libpu.h>

using namespace std;

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
static string opt_user;

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

	if (opt_user.empty())
		return 0;

	while ((s = gr->gr_mem[i++]) != NULL)
		if (!strcmp(s, opt_user.c_str()))
			return 1;

	return 0;
}

class grpent {
public:
	gid_t		gid;
	string		name;

	grpent(gid_t gid_, const string& name_) : gid(gid_), name(name_) {}
};

static bool match_groups(gid_t gid, gid_t egid, vector<grpent>& rv)
{
	bool rc = true;
	rv.clear();

	struct group *gr;

	setgrent();

	while (1) {
		errno = 0;
		gr = getgrent();
		if (!gr) {
			if (errno) {
				perror("getgrent");
				rc = false;
			}
			break;
		}

		if (gr->gr_gid == gid || gr->gr_gid == egid ||
		    user_in_group(gr)) {
			grpent ge(gr->gr_gid, gr->gr_name);
			rv.push_back(ge);
		}
	}

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
			printf("%s%s%s",
			       space ? " " : "",
			       pw->pw_name,
			       newline ? "\n" : "");
			return;
		}
	}

	uid_ull = uid;
	printf("%s%Lu%s",
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
			printf("%s%s%s",
			       space ? " " : "",
			       gr->gr_name,
			       newline ? "\n" : "");
			return;
		}
	}

	gid_ull = gid;
	printf("%s%Lu%s",
	       space ? " " : "",
	       gid_ull,
	       newline ? "\n" : "");
}

static int id_grp_all(vector<grpent>& grp, gid_t gid, gid_t egid)
{
	const char *s;
	char s1[64];

	for (unsigned int i = 0; i < grp.size(); i++) {
		grpent& tmp = grp[i];

		if (opt_name)
			s = tmp.name.c_str();
		else {
			snprintf(s1, sizeof(s1), "%Lu",
				 (unsigned long long) tmp.gid);
			s = s1;
		}

		printf("%s%s%s",
		       (i == 0) ? "" : " ",
		       s,
		       (i == (grp.size() - 1)) ? "\n" : "");
	}

	return 0;
}

static int id_def(uid_t uid, uid_t euid, gid_t gid, gid_t egid,
		  vector<grpent>& grp)
{
	struct passwd *pw;
	struct group *gr;

	bool have_opt_user = !opt_user.empty();
	printf("uid=%Lu%s%s%s",
	       (unsigned long long) uid,
	       have_opt_user ? "(" : "",
	       have_opt_user ? opt_user.c_str() : "",
	       have_opt_user ? ")" : "");

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

	if (!grp.empty()) {
		printf(" groups=");

		for (unsigned int i = 0; i < grp.size(); i++) {
			grpent& tmp = grp[i];

			printf("%s%Lu(%s)",
			       (i == 0) ? "" : ",",
			       (unsigned long long) tmp.gid,
			       tmp.name.c_str());
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

	if (!opt_user.empty()) {
		errno = 0;
		pw = getpwnam(opt_user.c_str());
		if (!pw) {
			if (errno)
				perror(opt_user.c_str());
			else
				fprintf(stderr, "user '%s' not found\n",
					opt_user.c_str());
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
				opt_user.assign(pw->pw_name);
		}
	}

	switch (opt_mode) {
	case ID_EUID:	pr_uid(euid, false, true); return 0;
	case ID_EGID:	pr_gid(egid, false, true); return 0;
	default:	break;
	}

	vector<grpent> grp;
	if (!match_groups(gid, egid, grp))
		return 1;

	switch (opt_mode) {
	case ID_GRP_ALL:	return id_grp_all(grp, gid, egid);
	case ID_DEF:		return id_def(uid, euid, gid, egid, grp);
	default:		return 1;
	}
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

	return do_id();
}

