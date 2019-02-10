
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

#include <string.h>
#include <argp.h>
#include <libpu.h>

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

