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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libpu.h>

int parse_cmdline(struct cmdline_walker *cw)
{
	error_t rc_argp;
	int idx = 0;

	rc_argp = argp_parse(cw->argp, cw->argc, cw->argv, 0, &idx, NULL);
	if (rc_argp) {
		fprintf(stderr, "argp_parse: %s\n", strerror(rc_argp));
		return -rc_argp;
	}

	return idx;
}

int __walk_cmdline(struct cmdline_walker *cw, int idx)
{
	int rc, err = 0;

	if (idx >= cw->argc) {	/* no arguments */
		if (cw->flags & CFL_NO_FILES_STDIN) {
			if (cw->fd_actor)
				rc = cw->fd_actor(cw, STDIN_NAME, STDIN_FILENO);
			else
				rc = cw->fn_actor(cw, NULL);
			return rc;
		}

		return CERR_NO_ARGS;
	}

	while (idx < cw->argc) {
		const char *fn;

		if ((cw->flags & CFL_SKIP_LAST) && (idx == (cw->argc - 1)))
			break;
		fn = cw->argv[idx];
		if ((cw->flags & CFL_STDIN_DASH) && (!strcmp(fn, "-")))
			err |= cw->fd_actor(cw, STDIN_NAME, STDIN_FILENO);
		else
			err |= cw->fn_actor(cw, fn);

		idx++;
	}

	return err;
}

int walk_cmdline(struct cmdline_walker *cw)
{
	int rc;

	rc = parse_cmdline(cw);
	if (rc < 0)
		return -rc;

	return __walk_cmdline(cw, rc);
}

