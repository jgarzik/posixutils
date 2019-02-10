
/*
 * Copyright 2019 Bloq Inc.
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

#include <vector>
#include <string>
#include <regex>
#include <argp.h>
#include <libpu.h>

using namespace std;

static const char doc[] =
N_("expand - convert tabs to spaces");

static struct argp_option options[] = {
	{ "tabs", 't', "tablist", 0,
	  N_("Specify the tab stops. A single positive integer, or comma/blank-separated positive integers.") },
	{ }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, file_args_doc, doc };

class ExpandApp : public CmdlineApp {
private:
	unsigned int		repeated_tab;
	unsigned int		last_tab;
	Bitmap			tab_map;

public:
	ExpandApp() : CmdlineApp(true), repeated_tab(8) { }

	int run() { return run_arg_files(); }
	int arg_file(StdioFile& f);

	bool set_tablist(const std::string& liststr);

private:
	void outspace(unsigned int& column);
	void advance_rtab(unsigned int& column);
	void advance_tablist(unsigned int& column);
};

static regex rxInteger("^(\\d+)$");

bool ExpandApp::set_tablist(const std::string& liststr)
{
	vector<string> sv;

	// branch: tab "list" is single positive integer
	if (regex_match(liststr, rxInteger)) {
		int tmp = atoi(liststr.c_str());
		if (tmp < 1)
			return false;
		last_tab = repeated_tab = tmp;
		return true;
	}
	
	// split based on comma, if present, blanks otherwise
	if (liststr.find(',')) {
		strsplit(liststr, ',', sv);
	} else
		strsplit(liststr, sv);

	// require at least two tablist entries
	if (sv.size() < 2)
		return false;

	// validate each tablist entry
	int last = 0;
	for (auto it = sv.begin(); it != sv.end(); it++) {
		// must be positive, consecutive integer
		int cur = atoi((*it).c_str());
		if ((cur < 1) || (cur <= last))
			return false;

		tab_map.set(cur);
		last_tab = cur;

		last = cur;
	}

	// we have a tablist; disable repeated-tab feature
	repeated_tab = 0;

	return true;
}

void ExpandApp::outspace(unsigned int& column)
{
	putchar(' ');
	column++;
}

void ExpandApp::advance_rtab(unsigned int& column)
{
	while ((column % repeated_tab) != 0)
		outspace(column);
	outspace(column);
}

void ExpandApp::advance_tablist(unsigned int& column)
{
	if (column >= last_tab) {
		outspace(column);
		return;
	}

	while (column < last_tab) {
		outspace(column);
		if (tab_map.test(column))
			break;
	}
	outspace(column);
}

int ExpandApp::arg_file(StdioFile& f)
{
	unsigned int column = 1;
	while (!f.eof()) {
		int ch = f.getc();
		if (ch == EOF)
			break;

		if (ch == '\b') {
			putchar(ch);
			if (column > 1)
				column--;
		} else if (ch == '\r') {
			putchar(ch);
			column = 1;
		} else if (ch == '\n') {
			putchar(ch);
			column = 1;
		} else if (ch != '\t') {
			putchar(ch);
			column++;
		} else if (repeated_tab)
			advance_rtab(column);
		else
			advance_tablist(column);
	}

	return (f.err() ? 1 : 0);
}

static ExpandApp app;

#define PU_OPT_PUSH_ARG \
	case ARGP_KEY_ARG: app.push_arg(arg); break;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN

	case 't':
		if (!app.set_tablist(arg))
			return ARGP_ERR_UNKNOWN;
		break;
		
	PU_OPT_PUSH_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

int main (int argc, char *argv[])
{
	return app.init_and_run(&argp, argc, argv);
}

