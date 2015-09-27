
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <signal.h>
#include <libpu.h>

using namespace std;

enum {
	NOHUP_ERR		= 127,
};

#define OUTPUT_FN "nohup.out"

static int redirect_fd = STDOUT_FILENO;


static void redirect_stdout(void)
{
	const char *home_env = getenv("HOME");
	string fn = OUTPUT_FN;

	int fd = open(fn.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if ((fd < 0) && home_env) {
		fn = home_env + string("/") + fn;
		fd = open(fn.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	}
	if (fd < 0) {
		perror(fn.c_str());
		exit(NOHUP_ERR);
	}

	if (dup2(fd, STDOUT_FILENO) < 0) {
		perror(_("stdout redirect failed"));
		exit(NOHUP_ERR);
	}

	fprintf(stderr, _("nohup: appending output to '%s'\n"), fn.c_str());

	redirect_fd = fd;
}

static void redirect_stderr(void)
{
	if (dup2(redirect_fd, STDERR_FILENO) < 0) {
		/* if stderr redirect failed, this message might be lost */
		perror(_("stderr redirect failed"));
		exit(NOHUP_ERR);
	}
}

int main (int argc, char **argv)
{
	char **exec_argv = argv + 1;

	pu_init();

	if (argc < 2) {
		fprintf(stderr, _("nohup: no arguments\n"));
		return NOHUP_ERR;
	}

	if (isatty(STDOUT_FILENO))
		redirect_stdout();
	if (isatty(STDERR_FILENO))
		redirect_stderr();

	signal(SIGHUP, SIG_IGN);

	execvp(*exec_argv, exec_argv);

	/* if we are still here, we got an error */
	perror(_("execvp(3)"));
	return NOHUP_ERR;
}

