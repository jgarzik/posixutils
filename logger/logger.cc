/*
 * Copyright 2004-2015 Jeff Garzik <jgarzik@pobox.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string>
#include <libpu.h>


int main (int argc, char *argv[])
{
	pu_init();

	if (argc < 2) {
		fprintf(stderr, _("%s: no arguments\n"), argv[0]);
		return EXIT_FAILURE;
	}

	std::string s(argv[1]);
	for (int i = 2; i < argc; i++) {
		s.append(" ");
		s.append(argv[i]);
	}

	syslog(LOG_USER | LOG_NOTICE, "%s", s.c_str());

	return EXIT_SUCCESS;
}
