
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <vector>
#include <set>
#include <argp.h>
#include <libpu.h>

using namespace std;

enum {
	N_DIFFS = 2,
	HASH_START = 0x67502139,
};

static const char doc[] =
N_("diff - compare two files");

static const char args_doc[] = N_("file1 file2");
static struct argp_option options[] = {
	{ NULL, 'b', NULL, 0,
	  N_("Compare streams of blanks as equivalent") },
	{ NULL, 'c', NULL, 0,
	  N_("Context diff output, 3 lines") },
	{ NULL, 'C', "N", 0,
	  N_("Context diff output, N lines") },
	{ NULL, 'e', NULL, 0,
	  N_("ed output") },
	{ NULL, 'f', NULL, 0,
	  N_("Alterna-ed output") },
	{ NULL, 'r', NULL, 0,
	  N_("Apply diff recursively to directories") },
	{ }
};

/* "djb2"-derived hash function */
static unsigned long blob_hash(unsigned long hash, const void *_buf, size_t buflen)
{
	const unsigned char *buf = (const unsigned char *) _buf;
	int c;

	while (buflen > 0) {
		c = *buf++;
		buflen--;

		hash = ((hash << 5) + hash) ^ c; /* hash * 33 ^ c */
	}

	return hash;
}

class AutoFile {
public:
	FILE		*fp;

	AutoFile(FILE *f_) {
		fp = f_;
	}
	AutoFile(string filename) {
		fp = NULL;
		if (ro_file_open(&fp, filename.c_str()) != 0)
			fp = NULL;
	}
	~AutoFile() {
		fclose(fp);
	}

	bool isOpen() { return (fp != NULL); }
	bool haveError() { return (ferror(fp)); }
};

class DiffLine {
public:
	string data;
	unsigned long hash;

	DiffLine(string data_) {
		data = data_;
		hash = blob_hash(HASH_START, &data[0], data.size());
	}
};

class DiffFile {
public:
	string filename;
	vector<DiffLine> lines;
	set<unsigned long> haveHash;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static bool opt_recurse;
static bool opt_blanks_equiv;
static unsigned int opt_cdiff_lines = 3;
static enum opt_mode_type {
	MODE_CONTEXT,
	MODE_ED,
	MODE_REVERSE_ED,
} opt_mode;
static DiffFile dfiles[N_DIFFS];

static const struct argp argp = { options, parse_opt, args_doc, doc };

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'b':
		opt_blanks_equiv = true;
		break;
	case 'c':
		opt_mode = MODE_CONTEXT;
		opt_cdiff_lines = 3;
		break;
	case 'C':
		opt_mode = MODE_CONTEXT;
		opt_cdiff_lines = atoi(arg);
		break;
	case 'e':
		opt_mode = MODE_ED;
		break;
	case 'f':
		opt_mode = MODE_REVERSE_ED;
		break;
	case 'r':
		opt_recurse = true;
		break;

	case ARGP_KEY_ARG:
		switch (state->arg_num) {
		case 0:
			dfiles[0].filename.assign(arg);
			break;
		case 1:
			dfiles[1].filename.assign(arg);
			break;
		default:
			argp_usage (state);	/* too many args */
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static int read_file(DiffFile& dfile)
{
	AutoFile f(dfile.filename);
	if (!f.isOpen())
		return 1;

	char *line, buf[LINE_MAX + 1];

	while ((line = fgets_unlocked(buf, sizeof(buf), f.fp)) != NULL) {
		buf[sizeof(buf) - 1] = 0;

		DiffLine ln(line);
		dfile.lines.push_back(ln);
		dfile.haveHash.insert(ln.hash);
	}

	if (f.haveError())
		return 1;

	return 0;
}

static int do_diff(void)
{
	for (unsigned int i = 0; i < N_DIFFS; i++)
		if (read_file(dfiles[i]) != 0)
			return 1;

	// TODO

	return 0;
}

int main (int argc, char *argv[])
{
	error_t rc;

	pu_init();

	rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (rc) {
		fprintf(stderr, "argp_parse failed: %s\n", strerror(rc));
		return 1;
	}

	return do_diff();
}

