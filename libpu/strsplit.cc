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
#include <regex.h>
#include <libpu.h>

void strsplit(const std::string& s, std::vector<std::string>& sv)
{
	sv.clear();

	std::string tmp;

	bool in_string = true;
	unsigned int cursor = 0;

	while (cursor < s.size()) {
		int ch = s[cursor++];
		if (isspace(ch)) {
			if (in_string) {
				sv.push_back(tmp);
				tmp.clear();
				in_string = false;
			}
		} else {
			tmp.append(1, ch);
			in_string = true;
		}
	}

	if (in_string)
		sv.push_back(tmp);
}

void strsplit(const std::string& s, int delim, std::vector<std::string>& sv)
{
	sv.clear();

	std::string tmp;

	bool in_string = true;
	unsigned int cursor = 0;

	while (cursor < s.size()) {
		int ch = s[cursor++];
		if (ch == delim) {
			sv.push_back(tmp);
			tmp.clear();
			in_string = false;
		} else {
			tmp.append(1, ch);
			in_string = true;
		}
	}

	if ((s.size() > 0) && in_string)
		sv.push_back(tmp);
}

void strsplit(const std::string& s, const std::string& regex,
	      std::vector<std::string>& sv)
{
	Regex rx(regex, REG_EXTENDED);
	if (!rx.ok())
		return;

	sv.clear();

	size_t nmatch = 4;
	regmatch_t matches[nmatch];
	std::string tmp(s);

	while (tmp.size() > 0) {
		bool rcm = rx.match(tmp, nmatch, matches);
		if (!rcm) {
			sv.push_back(tmp);
			break;
		}

		// push string prior to delimiter
		sv.push_back(tmp.substr(0, matches[0].rm_so));

		// grab string following delimiter
		tmp = tmp.substr(matches[0].rm_eo);
	}
}

void strbisect(const std::string& s, int delim,
	       std::string& s1, std::string& s2)
{
	size_t pos = s.find((char)delim);
	if (pos == std::string::npos) {
		s1.assign(s);
		s2.clear();
	} else {
		s1 = s.substr(0, pos);
		s2 = s.substr(pos + 1);
	}
}

