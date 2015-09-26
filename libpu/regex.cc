/*
 * Copyright 2015-2006 Jeff Garzik <jgarzik@pobox.com>
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
#include <regex.h>
#include <libpu.h>

using namespace std;

bool Regex::match(const string& haystack, string& out1, int eflags)
{
	if (!initd)
		return false;

	static const unsigned int max_match = 1 + 1;
	regmatch_t matchv[max_match];

	int rc = regexec(&reg, haystack.c_str(),
			 max_match, matchv, eflags);
	if (rc || (matchv[1].rm_so == -1))
		return false;

	out1 = haystack.substr(matchv[1].rm_so,
			       matchv[1].rm_eo - matchv[1].rm_so);

	return true;
}

bool Regex::match(const string& haystack, string& out1, string& out2, int eflags)
{
	if (!initd)
		return false;

	static const unsigned int max_match = 2 + 1;
	regmatch_t matchv[max_match];

	int rc = regexec(&reg, haystack.c_str(),
			 max_match, matchv, eflags);
	if (rc || (matchv[2].rm_so == -1))
		return false;

	out1 = haystack.substr(matchv[1].rm_so,
			       matchv[1].rm_eo - matchv[1].rm_so);
	out2 = haystack.substr(matchv[2].rm_so,
			       matchv[2].rm_eo - matchv[2].rm_so);

	return true;
}

bool Regex::match(const string& haystack, string& out1, string& out2, string& out3, int eflags)
{
	if (!initd)
		return false;

	static const unsigned int max_match = 3 + 1;
	regmatch_t matchv[max_match];

	int rc = regexec(&reg, haystack.c_str(),
			 max_match, matchv, eflags);
	if (rc || (matchv[3].rm_so == -1))
		return false;

	out1 = haystack.substr(matchv[1].rm_so,
			       matchv[1].rm_eo - matchv[1].rm_so);
	out2 = haystack.substr(matchv[2].rm_so,
			       matchv[2].rm_eo - matchv[2].rm_so);
	out3 = haystack.substr(matchv[3].rm_so,
			       matchv[3].rm_eo - matchv[3].rm_so);

	return true;
}

