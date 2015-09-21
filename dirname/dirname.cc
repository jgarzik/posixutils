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

#include <limits.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libpu.h>


static char pathbuf[PATH_MAX + 2];


int main (int argc, char *argv[])
{
	char *dn;

	pu_init();

	if (argc != 2) {
		fprintf(stderr, _("invalid number of arguments\n"));
		return 1;
	}

	strncpy(pathbuf, argv[1], sizeof(pathbuf));
	pathbuf[sizeof(pathbuf) - 1] = 0;

	if (strlen(pathbuf) > PATH_MAX) {
		fprintf(stderr, _("invalid pathname\n"));
		return 1;
	}

	dn = dirname(pathbuf);
	printf("%s\n", dn);

	return 0;
}

