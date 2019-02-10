
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

class Bitmap {
private:
	std::vector<uint32_t>	uv;

public:
	Bitmap(size_t initsz = 64) : uv((initsz / 32) + 1) {}
	
	bool test(size_t idx) {
		size_t bucket = idx / 32;
		size_t bit = idx % 32;

		if (bucket >= uv.size())
			return false;

		return (uv[bucket] & (1U << bit));
	}

	void set(size_t idx) {
		size_t bucket = idx / 32;
		size_t bit = idx % 32;

		size_t req_size = bucket + 1;
		if (uv.size() < req_size)
			uv.resize(req_size);

		uv[bucket] = uv[bucket] | (1U << bit);
	}

	void clear(size_t idx) {
		size_t bucket = idx / 32;
		size_t bit = idx % 32;

		if (bucket >= uv.size())
			return;

		uv[bucket] = uv[bucket] & ~(1U << bit);
	}
};

class StdioFile {
private:
	std::string	pr_filename;
	FILE		*fp;
	bool		have_error;
	bool		at_eof;

public:
	bool eof() const { return at_eof; }
	bool err() const { return have_error; }
	bool is_open() const { return (fp != nullptr); }
	const std::string& pr_fn() const { return pr_filename; }
	FILE *get_fp() { return fp; }

	StdioFile() : fp(nullptr), have_error(false), at_eof(false) {}
	~StdioFile() { close(); }

	void openStdin()
	{
		pr_filename = _("(standard input)");
		fp = stdin;
		at_eof = false;
		have_error = false;
	}
	void openStdout()
	{
		pr_filename = _("(standard output)");
		fp = stdout;
		at_eof = false;
		have_error = false;
	}

	void setflags()
	{
		if (ferror(fp))
			have_error = true;
		else if (feof(fp))
			at_eof = true;
	}

	bool open(const std::string& filename, const std::string& mode = "r") {
		pr_filename = filename;

		fp = ::fopen(filename.c_str(), mode.c_str());
		if (!fp) {
			have_error = true;
			return false;
		}

		at_eof = false;
		have_error = false;
		return true;
	}
	void close() {
		if (!fp)
			return;

		::fclose(fp);
		fp = nullptr;
	}

	int getc() {
		int rc = ::fgetc(fp);
		if (rc == EOF)
			setflags();
		return rc;
	}
	bool gets(std::string& retStr, size_t maxsz = 1024) {
		if (!fp)
			return false;

		char linebuf[maxsz + 1];
		char *rp = ::fgets(linebuf, sizeof(linebuf), fp);
		if (!rp) {
			setflags();
			return false;
		}
		retStr.assign(linebuf);
		return true;
	}

	size_t read(void *ptr, size_t sz) {
		if (!fp || !ptr)
			return 0;

		size_t rc = ::fread(ptr, 1, sz, fp);
		if (rc < sz)
			setflags();
		return rc;
	}

	bool write(const void *ptr, size_t sz) {
		if (!fp || !ptr)
			return false;

		size_t rc = ::fwrite(ptr, 1, sz, fp);
		if (rc != sz) {
			have_error = true;
			return false;
		}

		return true;
	}
};

class CmdlineApp {
private:
	std::vector<std::string> arglist;
	bool stdin_or_files;

public:
	CmdlineApp(bool sof_ = false) : stdin_or_files(sof_) {}
	virtual ~CmdlineApp() {}

	void push_arg(const std::string& arg) { arglist.push_back(arg); }

	virtual int arg_file(StdioFile& f) { return 1; }

	int init_and_run(const struct argp *argp, int argc, char **argv);
	virtual int init(const struct argp *argp, int argc, char **argv);
	virtual int run() { return 0; }
	int run_arg_files();
};

int CmdlineApp::init(const struct argp *argp, int argc, char **argv)
{
	pu_init();

	error_t argp_rc = argp_parse(argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int CmdlineApp::init_and_run(const struct argp *argp, int argc, char **argv)
{
	int rc = init(argp, argc, argv);
	if (rc)
		return rc;
	
	return run();
}

int CmdlineApp::run_arg_files()
{
	// if no files, process stdin
	if (stdin_or_files && arglist.size() == 0) {
		StdioFile f;
		f.openStdin();
		return arg_file(f);
	}

	int rc = 0;

	for (auto it = arglist.begin(); it != arglist.end(); it++) {
		const std::string& fn = *it;

		StdioFile f;
		if (!f.open(fn)) {
			perror(fn.c_str());
			rc = 1;
		} else {
			int tmp_rc = arg_file(f);
			if (!rc)
				rc = tmp_rc;
		}
	}

	return rc;
}

class ExpandApp : public CmdlineApp {
private:
	unsigned int		repeated_tab;
	unsigned int		last_tab;
	std::vector<unsigned int> tabs;
	Bitmap			tab_map;

public:
	ExpandApp() : CmdlineApp(true), repeated_tab(8) { }

	int run() { return run_arg_files(); }

	int arg_file(StdioFile& f);

	bool set_tablist(const std::string& liststr);

	void outspace(unsigned int& column);
	void advance_rtab(unsigned int& column);
	void advance_tablist(unsigned int& column);
};

static regex rxInteger("^(\\d+)$");

bool ExpandApp::set_tablist(const std::string& liststr)
{
	vector<string> sv;

	if (regex_match(liststr, rxInteger)) {
		int tmp = atoi(liststr.c_str());
		if (tmp < 1)
			return false;
		last_tab = repeated_tab = tmp;
		return true;
	}
	
	if (liststr.find(',')) {
		strsplit(liststr, ',', sv);
	} else
		strsplit(liststr, sv);

	if (sv.size() < 2)
		return false;

	int last = 0;
	for (auto it = sv.begin(); it != sv.end(); it++) {
		int cur = atoi((*it).c_str());
		if ((cur < 1) || (cur <= last))
			return false;

		tabs.push_back(cur);
		tab_map.set(cur);
		last_tab = cur;

		last = cur;
	}

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

