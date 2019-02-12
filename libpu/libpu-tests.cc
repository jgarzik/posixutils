
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

#include <string>
#include <vector>
#include <libpu.h>
#include <assert.h>

using namespace std;

static void test_strsplit()
{
	vector<string> sv;

	strsplit("a b c", sv);
	assert(sv.size() == 3);
	assert(sv[0] == "a");
	assert(sv[1] == "b");
	assert(sv[2] == "c");

	strsplit(" b c", sv);
	assert(sv.size() == 3);
	assert(sv[0] == "");
	assert(sv[1] == "b");
	assert(sv[2] == "c");

	strsplit("a b c\n", sv);
	assert(sv.size() == 3);
	assert(sv[0] == "a");
	assert(sv[1] == "b");
	assert(sv[2] == "c");

	strsplit("a:b:c", ':', sv);
	assert(sv.size() == 3);
	assert(sv[0] == "a");
	assert(sv[1] == "b");
	assert(sv[2] == "c");

	strsplit("a  b     c", "[[:space:]]+", sv);
	assert(sv.size() == 3);
	assert(sv[0] == "a");
	assert(sv[1] == "b");
	assert(sv[2] == "c");

	string s1, s2;

	strbisect("a::", ':', s1, s2);
	assert(s1 == "a");
	assert(s2 == ":");

	strbisect("a", ':', s1, s2);
	assert(s1 == "a");
	assert(s2 == "");

	strbisect(":a", ':', s1, s2);
	assert(s1 == "");
	assert(s2 == "a");

	strbisect("a:", ':', s1, s2);
	assert(s1 == "a");
	assert(s2 == "");
}

int main (int argc, char *argv[])
{
	test_strsplit();
	return 0;
}
