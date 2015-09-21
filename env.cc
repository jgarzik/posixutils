
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

#include <unistd.h>
#include <string.h>
#include <libpu.h>

static const char doc[] =
N_("env - set the environment for command invocation");

static const char args_doc[] = N_("[name=value]... utility [args]...");

static struct argp_option options[] = {
	{ NULL, 'i', NULL, 0,
	  N_("Invoke utility with exactly the environment specified by the arguments") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static bool opt_no_inherit;

static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct strlist env_list, arg_list;
static bool parsing_env = true;
extern char **environ;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'i':
		opt_no_inherit = true;
		break;

	case ARGP_KEY_ARG:
		if (parsing_env &&
		    (!strchr(arg, '=') || (*arg == '=')))
			parsing_env = false;
		if (parsing_env)
			slist_push(&env_list, arg, false);
		else
			slist_push(&arg_list, arg, false);
		break;

	case ARGP_KEY_END:
		if (opt_no_inherit && env_list.len == 0) /* no env */
			argp_usage (state);
		if (arg_list.len == 0)		/* not enough args */
			argp_usage (state);
		return ARGP_ERR_UNKNOWN;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int count_env(void)
{
	int i = 0;
	while (environ[i])
		i++;
	return i;
}

static bool env_listed(const char *s)
{
	const char *end, *s2, *end2;
	size_t slen, slen2, i;

	end = strchr(s, '=');
	if (!end)
		return false;

	slen = end - s;
	if (!slen)
		return false;

	for (i = 0; i < env_list.len; i++) {
		s2 = env_list.list[i].s;
		end2 = strchr(s2, '=');
		slen2 = end2 - s2;
		if ((slen == slen2) &&
		    (!strncmp(s, s2, slen)))
			return true;
	}

	return false;
}

static int do_env(void)
{
	char **env, **arg, *msg;
	int n_env, i, j;

	arg = (char **) xmalloc((arg_list.len + 1) * sizeof(char *));
	for (i = 0; i < arg_list.len; i++)
		arg[i] = (char *) arg_list.list[i].s;
	arg[i] = NULL;

	n_env = env_list.len + 1;
	if (!opt_no_inherit)
		n_env += count_env();

	env = (char **) xmalloc(n_env * sizeof(char *));
	memset(env, 0, n_env * sizeof(char *));

	for (i = 0; i < env_list.len; i++)
		env[i] = (char *) env_list.list[i].s;

	j = 0;
	while (environ[j]) {
		if (!env_listed(environ[j]))
			env[i++] = environ[j];
		j++;
	}

	environ = env;
	execvp(arg[0], arg);

	msg = (char *) xmalloc(strlen(arg[0]) + 16);
	sprintf(msg, _("execv(%s)"), arg[0]);
	perror(msg);
	return 1;
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

	return do_env();
}

