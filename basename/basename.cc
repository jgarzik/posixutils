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

#include <stdio.h>
#include <libgen.h>
#include <string>
#include <libpu.h>

int main (int argc, char *argv[])
{
	std::string suffix;

	pu_init();

	// basename PATH
	if (argc == 2) {
		// do nothing

	// basename PATH SUFFIX
	} else if (argc == 3)
		suffix.assign(argv[2]);

	// too few or too many args
	else {
		fprintf(stderr, _("Usage: %s PATH [SUFFIX]\n"), argv[0]);
		return 1;
	}

	// take input path, strip directories using basename(3)
	// basename(3) may modify the string passed to it, so
	// we pass a copy (pathbuf).
	std::string pathbuf(argv[1]);
	std::string bn(basename(&pathbuf[0]));

	// strip suffix, if match present
	size_t suffix_len = suffix.size();
	size_t trunc_len = bn.size() - suffix_len;

	if ((suffix_len > 0) &&
	    (suffix_len < bn.size()) &&
	    (suffix == bn.substr(trunc_len)))
		bn.resize(trunc_len);

	// final processed output
	printf("%s\n", bn.c_str());

	return 0;
}

