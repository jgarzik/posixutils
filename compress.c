/*
 * Copyright 2004-2005 Jeff Garzik <jgarzik@pobox.com>
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
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <libpu.h>
#include <libcompress.h>


#define PFX "compress: "

static const char doc[] =
N_("compress - compress data");

#define args_doc file_args_doc

static struct argp_option options[] = {
	{ "bits", 'b', "N", 0,
	  N_("Specify the maximum number of bits to use in a code.  Supported values from 9 to 14, inclusive.") },
	{ "stdout", 'c', NULL, 0,
	  N_("Cause compress to write to the standard output") },
	{ "force", 'f', NULL, 0,
	  N_("Force compression of file, even if it does not actually reduce the size of the file, or if the corresponding .Z file already exists") },
	{ "verbose", 'v', NULL, 0,
	  N_("Write the percentage reduction of each file to standard error.") },
	{ }
};

static int compress_fn_actor(struct walker *w, const char *fn, const struct stat *st);

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker = {
	.argp			= &argp,
	.flags			= WF_NO_FILES_STDIN,
	.cmdline_arg		= compress_fn_actor,
};

static bool opt_stdout;
static bool opt_force;
static bool opt_verbose;
static int opt_bits = COMPRESS_DEFBITS;
static int first_file_only;
static unsigned char outbuf[4096 * 3], inbuf[4096];


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_ARG
	PU_OPT_SET('v', verbose)
	PU_OPT_SET('f', force)

	case 'c':
		opt_stdout = true;
		first_file_only = 1;
		break;

	case 'b': {
		int rc, tmp;

		rc = sscanf(arg, "%d", &tmp);
		if ((rc != 1) ||
		    (tmp < COMPRESS_MINBITS) || (tmp > COMPRESS_MAXBITS))
			return ARGP_ERR_UNKNOWN;

		opt_bits = tmp;
		break;
	}

	PU_OPT_DEFAULT
	PU_OPT_END
}

static int do_file_header(struct compress *comp, int out_fd, const char *out_fn)
{
	outbuf[0] = COMPRESS_MAGIC1;
	outbuf[1] = COMPRESS_MAGIC2;
	outbuf[2] = comp->code_maxbits | comp->block_mode;

	if (write_fd(out_fd, outbuf, 3, out_fn))
		return 1;

	return 0;
}

static void do_init(struct compress *comp)
{
	memset(comp, 0, sizeof(*comp));

	comp->block_mode	= COMPRESS_BLOCK_MODE;
	comp->code_minbits	= COMPRESS_MINBITS;
	comp->code_maxbits	= opt_bits;

	comp->dict_add		= dict_add;
	comp->dict_lookup	= dict_lookup;
	comp->dict_clear	= dict_clear;
	comp->dict_new		= dict_new;
	comp->dict_free		= dict_free;

	if (compress_init(comp) < 0)
		die(_("out of memory"));
}

static int do_compression(struct compress *comp,
			  int in_fd, const char *in_fn,
			  int out_fd, const char *out_fn)
{
	while (1) {
		ssize_t rrc = read(in_fd, inbuf, sizeof(inbuf));
		if (rrc < 0) {
			perror(in_fn);
			return 1;
		}
		if (rrc == 0)
			break;

		comp->next_in = inbuf;
		comp->avail_in = rrc;

		while (comp->avail_in > 0) {
			comp->next_out = outbuf;
			comp->avail_out = sizeof(outbuf);

			compress_io(comp);

			if (comp->avail_out < sizeof(outbuf))
				if (write_fd(out_fd, outbuf,
					     sizeof(outbuf) - comp->avail_out,
					     out_fn))
					return 1;
		}

	}

	return 0;
}

static int compress_fn_actor(struct walker *w, const char *fn, const struct stat *lst)
{
	int tmp, have_stat = 0, rc = 0, in_fd = -1, out_fd = -1;
	char *nfn = NULL;
	struct compress comp;
	struct stat st;

	if (first_file_only && (first_file_only++ > 1))
		return 1;

	/* initialize input */
	if (fn != NULL) {
		in_fd = open(fn, O_RDONLY);
		if (in_fd < 0) {
			perror(fn);
			return 1;
		}

		/* Do we care about the return value of posix_fadvise(2) ? */
		posix_fadvise(in_fd, 0, 0, POSIX_FADV_SEQUENTIAL);

		if (fstat(in_fd, &st) < 0) {
			perror(fn);
			close(in_fd);
			return 1;
		}

		have_stat = 1;
	} else {
		in_fd = STDIN_FILENO;

		if (!isatty(in_fd) && !fstat(in_fd, &st))
			have_stat = 1;
	}

	/* initialize output */
	if (opt_stdout)
		out_fd = STDOUT_FILENO;
	else {
		size_t len;
		int flags = O_CREAT | O_TRUNC | O_WRONLY;

		len = strlen(fn) + 3;
		if (len > NAME_MAX) {
			fprintf(stderr, _("filename too long:\n%s\n"), fn);
			rc = 1;
			goto out;
		}

		nfn = xmalloc(len);

		sprintf(nfn, "%s.Z", fn);

		if (!opt_force)
			flags |= O_EXCL;

open_output:
		out_fd = open(nfn, flags, 0666);
		if (out_fd < 0) {
			if ((!opt_force) && (flags & O_EXCL) &&
			    (errno == EEXIST) && isatty(STDIN_FILENO)) {
				int qrc = ask_question(PFX,
						"%soverwrite %s? ", nfn);
				if (!qrc) {
					rc = 1;
					goto out;
				}

				flags &= ~O_EXCL;
				goto open_output;
			}

			perror(nfn);
			rc = 1;
			goto out;
		}
	}

	/*
	 * initialize compression
	 */
	do_init(&comp);

	/*
	 * output file header
	 */
	if (do_file_header(&comp, out_fd, nfn ? nfn : "<stdout>")) {
		comp.next_out = outbuf;
		comp.avail_out = sizeof(outbuf);
		tmp = compress_fini(&comp);
		assert(tmp == 0);

		rc = 1;
		goto out;
	}

	/*
	 * compress
	 */
	rc |= do_compression(&comp,
			     in_fd, fn ? fn : "<stdin>",
			     out_fd, nfn ? nfn : "<stdout>");

	/*
	 * compression trailer
	 */
	comp.next_out = outbuf;
	comp.avail_out = sizeof(outbuf);

	tmp = compress_fini(&comp);
	assert(tmp == 0);

	if (write_fd(out_fd, outbuf, sizeof(outbuf) - comp.avail_out,
		     nfn ? nfn : "<stdout>")) {
		rc = 1;
		goto out;
	}

	if (in_fd != STDIN_FILENO) {
		if (close(in_fd) < 0)
			perror(fn);
		if (unlink(fn) < 0)
			perror(fn);
		in_fd = -1;
	}

	if (opt_verbose) {
		if ((comp.total_in & 0xffffffff00000000ULL) ||
		    (comp.total_out & 0xffffffff00000000ULL)) {
			comp.total_in >>= 32;
			comp.total_out >>= 32;
		}
		long double in = comp.total_in;
		long double out = comp.total_out;
		long double ratio = 100 - ((out * 100) / in);
		const char *verb = "saved";

		if (ratio < 0.0) {
			ratio = fabsl(ratio);
			verb = "lost";
		}

		fprintf(stderr, _("%s: %2.2Lf %s, %" PRIuMAX " in, %" PRIuMAX " out.\n"),
			fn, ratio, verb,
			comp.total_in,
			comp.total_out);
	}

	if (have_stat && !isatty(out_fd))
		if (fchmod(out_fd, st.st_mode) < 0)
			perror(nfn);

out:
	if ((in_fd >= 0) && (in_fd != STDIN_FILENO))
		if (close(in_fd) < 0)
			perror(fn);
	if ((out_fd >= 0) && (out_fd != STDOUT_FILENO)) {
		if (close(out_fd) < 0)
			perror(nfn);
		if (rc && unlink(nfn) < 0)
			perror(nfn);
	}
	free(nfn);
	return rc;
}

int main (int argc, char *argv[])
{
	return walk(&walker, argc, argv);
}

