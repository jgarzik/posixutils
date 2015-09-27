/*
 * Copyright 2015 Jeff Garzik <jgarzik@pobox.com>
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
#include <stdlib.h>
#include <libgen.h>
#include <libpu.h>

using namespace std;

static bool componentize(const string& path, string& dirn, string& basen)
{
	string dirc(path);
	string basec(path);
	dirn = dirname(&dirc[0]);
	basen = basename(&basec[0]);
	return true;
}

bool path_split(const string& pathname, pathelem& pe)
{
	return componentize(pathname, pe.dirn, pe.basen);
}

