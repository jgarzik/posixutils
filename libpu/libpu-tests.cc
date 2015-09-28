
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

static void test_Regex()
{
	Regex t1;
	assert(t1.ok() == false);

	Regex t2("robin");
	assert(t2.ok() == true);
	assert(t2.match("red robin") == true);

	Regex t3("[[:space:]]+", REG_NOSUB | REG_EXTENDED);
	assert(t3.ok() == true);
	assert(t3.match("red robin") == true);

	string s1, s2, s3;

	Regex t4("^([^=]+)=.*$", REG_EXTENDED);
	assert(t4.ok() == true);
	assert(t4.match("name=bob", s2) == true);
	assert(s2 == "name");

	Regex t5("^([^=]+)=.*$", REG_EXTENDED);
	assert(t5.ok() == true);
	assert(t5.match("=bob", s2) == false);

	Regex t6("^([^=]+)=(.*)$", REG_EXTENDED);
	assert(t6.ok() == true);
	assert(t6.match("name=bob", s1, s2) == true);
	assert(s1 == "name");
	assert(s2 == "bob");

	Regex t7("^([^=]+)=([^;]+);(.*)$", REG_EXTENDED);
	assert(t7.ok() == true);
	assert(t7.match("age=32;64", s1, s2, s3) == true);
	assert(s1 == "age");
	assert(s2 == "32");
	assert(s3 == "64");

	size_t nmatch = 32;
	regmatch_t matches[nmatch];

	Regex t8("^([^=]+)=([^;]+);(.*)$", REG_EXTENDED);
	assert(t8.ok() == true);
	assert(t8.match("age=32;64", nmatch, matches) == true);
}

int main (int argc, char *argv[])
{
	test_strsplit();
	test_Regex();
	return 0;
}
