
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

#define __STDC_FORMAT_MACROS 1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>
#include <time.h>
#include <libpu.h>
#include "pax.h"


static const char doc[] =
N_("pax - read and write file archives and copy directory hierarchies");

static const char args_doc[] = N_("[file | pattern ...]");

static struct argp_option options[] = {
	{ NULL, 'r', NULL, 0,
	  N_("Read.") },
	{ NULL, 'w', NULL, 0,
	  N_("Write.") },
	{ NULL, 'a', NULL, 0,
	  N_("Append files to the end of the archive.") },
	{ NULL, 'b', "N", 0,
	  N_("Block the output at a positive decimal integer number of bytes per write to the archive file.") },
	{ NULL, 'c', NULL, 0,
	  N_("Match all file or archive members except those specified by the pattern or file operands.") },
	{ NULL, 'd', NULL, 0,
	  N_("Examine only directories themselves, not their contents.") },
	{ NULL, 'f', "FILE", 0,
	  N_("Specify the pathname of the input or output archive.") },
	{ NULL, 'H', NULL, 0,
	  N_("Follow symlinks on cmdline, rather than archiving them.") },
	{ NULL, 'i', NULL, 0,
	  N_("Interactively rename files or archive members.") },
	{ NULL, 'k', NULL, 0,
	  N_("Prevent the overwriting of existing files.") },
	{ NULL, 'l', NULL, 0,
	  N_("Source and destination files hardlinked, when possible.") },
	{ NULL, 'H', NULL, 0,
	  N_("Follow symlinks on cmdline or during recursion, rather than archiving them.") },
	{ NULL, 'n', NULL, 0,
	  N_("Select the first archive member that matches each pattern operand.") },
	{ NULL, 'o', "OPT1[,OPT2...]", 0,
	  N_("Set options.") },
	{ NULL, 'p', "string", 0,
	  N_("Specify one or more file characteristic options (privileges).") },
	{ NULL, 's', "replstr", 0,
	  N_("Modify file or archive member names named by pattern or file operands according to the substitution expression replstr.") },
	{ NULL, 't', NULL, 0,
	  N_("set the access time of each file read to the access time that it had before being read by pax.") },
	{ NULL, 'u', NULL, 0,
	  N_("Ignore files that are older than a pre-existing file or archive member with the same name.") },
	{ "verbose", 'v', NULL, 0,
	  N_("In list mode, produce a verbose table of contents. Otherwise, write archive member pathnames to standard error.") },
	{ NULL, 'x', "format", 0,
	  N_("Specify the output archive format.") },
	{ NULL, 'X', NULL, 0,
	  N_("When traversing the file hierarchy specified by a pathname, pax shall not descend into directories that have a different device ID (i.e. shall not cross filesystem boundaries).") },
	{ }
};

static int pax_noop_arg(struct walker *w, const char *fn, const struct stat *lst)
{
	return 0;
}
static int pax_pre_walk(struct walker *w);
static int pax_post_walk(struct walker *w);

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { options, parse_opt, args_doc, doc };

static struct walker walker;

enum pax_format_t {
	FMT_PAX,
	FMT_CPIO,
	FMT_USTAR,
	FMT_ZIP,
};

enum option_key_t {
	OKEY_DELETE,
	OKEY_EHDR_NAME,
	OKEY_GHDR_NAME,
	OKEY_INVALID,
	OKEY_LINKDATA,
	OKEY_LISTOPT,
	OKEY_TIMES,
};

enum pax_mode_t {
	PM_LIST,
	PM_READ,
	PM_WRITE,
	PM_COPY,
};

static bool opt_read;
static bool opt_write;
static bool opt_append;
static bool opt_invert_match;
static bool opt_recurse = true;
static bool opt_interactive_rename;
static bool opt_follow_links;
static bool opt_follow_links_cmdline;
static bool opt_overwrite = true;
static bool opt_hardlink;
static bool opt_first_only;
static bool opt_set_atime;
static bool opt_ignore_older;
static bool opt_verbose;
static bool opt_cross_filesys = true;
static const char *opt_archive_fn;	/* NULL == stdin/stdout */
static const char *opt_repl_str;
static enum pax_format_t opt_format = FMT_PAX;
static enum pax_mode_t opt_mode = PM_LIST;

struct pax_operations pax_ops;
unsigned int block_size = 0;


long octal_str(const char *s, int len)
{
	char octal[32], fmt[8];
	assert(len < 32);
	long l;

	strncpy(octal, s, len);
	octal[len] = 0;
	sprintf(fmt, "%%%dlo", len);
	if (sscanf(octal, fmt, &l) != 1)
		return -1;
	
	return l;
}

void pax_fi_clear(struct pax_file_info *fi)
{
	free(fi->pathname);
	free(fi->username);
	free(fi->groupname);
	free(fi->linkname);
	memset(fi, 0, sizeof(*fi));
}

#undef STRMAP

#define STRMAP optkeymap
#include "pax-options.h"
#undef STRMAP

static int reserved_key(const char *s)
{
	if (map_lookup(optkeymap, s) >= 0)
		return 1;
	return 0;
}

static error_t parse_arg_special(const char *key, const char *val)
{
	return ARGP_ERR_UNKNOWN; /* TODO */
}

static error_t parse_arg_keyval(const char *key, const char *val, bool per_file)
{
	return ARGP_ERR_UNKNOWN; /* TODO */
}

static error_t parse_arg_option(const char *s)
{
	char key[LINE_MAX + 1], val[LINE_MAX + 1];
	int rc;

	while ((*s) && isspace(*s))
		s++;

	if (sscanf(s, "%[0-9a-zA-Z_.-]:=%s", key, val) == 2)
		return parse_arg_keyval(key, val, true);

	rc = sscanf(s, "%[0-9a-zA-Z_.-]=%s", key, val);
	if (rc == 2) {
		if (reserved_key(key))
			return parse_arg_special(key, val);
		return parse_arg_keyval(key, val, false);
	}
	if ((rc == 1) && reserved_key(key))
		return parse_arg_special(key, NULL);

	return ARGP_ERR_UNKNOWN;
}

static error_t parse_arg_options(char *s)
{
	bool escape = false;
	char *out;
	error_t rc;
	char strbuf[LINE_MAX + 1];

	if (strlen(s) > LINE_MAX)
		return ARGP_ERR_UNKNOWN;

	out = strbuf;
	*out = 0;

	while (*s) {
		if (escape) {
			escape = false;
			*out++ = *s++;
		}
		else if (*s == '\\')
			escape = true;
		else if (*s == ',') {
			*out = 0;
			rc = parse_arg_option(strbuf);
			if (rc)
				return rc;
			out = strbuf;
			*out = 0;
		}
		else
			*out++ = *s++;
	}

	*out = 0;
	if (*strbuf) {
		rc = parse_arg_option(strbuf);
		if (rc)
			return rc;
	}

	return 0;
}

static error_t parse_arg_privs(const char *s)
{
	return ARGP_ERR_UNKNOWN; /* TODO */
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	PU_OPT_BEGIN
	PU_OPT_SET('r', read)
	PU_OPT_SET('w', write)
	PU_OPT_SET('a', append)
	PU_OPT_SET('c', invert_match)
	PU_OPT_SET('i', interactive_rename)
	PU_OPT_SET('l', hardlink)
	PU_OPT_SET('n', first_only)
	PU_OPT_SET('t', set_atime)
	PU_OPT_SET('u', ignore_older)
	PU_OPT_SET('v', verbose)

	case 'b': {
		unsigned int tmp;
		if (sscanf(arg, "%u", &tmp) != 1)
			return ARGP_ERR_UNKNOWN;
		if ((tmp < 1) || (tmp > (1024 * 1024)))
			return ARGP_ERR_UNKNOWN;
		block_size = tmp;
		break;
	}
	case 'd':
		opt_recurse = false;
		break;
	case 'f':
		opt_archive_fn = arg;
		break;
	case 'k':
		opt_overwrite = false;
		break;
	case 'H':
		opt_follow_links_cmdline = true;
		opt_follow_links = false;
		break;
	case 'L':
		opt_follow_links_cmdline = true;
		opt_follow_links = true;
		break;
	case 'o':
		return parse_arg_options(arg);
	case 'p':
		return parse_arg_privs(arg);
	case 's':
		opt_repl_str = arg;
		break;
	case 'x':
		if (!strcmp(arg, "pax"))
			opt_format = FMT_PAX;
		else if (!strcmp(arg, "cpio"))
			opt_format = FMT_CPIO;
		else if (!strcmp(arg, "ustar"))
			opt_format = FMT_USTAR;
		else if (!strcmp(arg, "zip"))
			opt_format = FMT_ZIP;
		else
			return ARGP_ERR_UNKNOWN;
		break;
	case 'X':
		opt_cross_filesys = false;
		break;

	PU_OPT_ARG
	PU_OPT_DEFAULT
	PU_OPT_END
}

static int pax_noop_file_end(struct pax_file_info *fi)
{
	return 0;
}

static int pax_noop_file_data(struct pax_file_info *fi, const char *buf,
			      size_t buflen)
{
	return 0;
}

size_t my_strftime(char *s, size_t max, const char *fmt, const struct tm *tm)
{
	return strftime(s, max, fmt, tm);
}

static int pax_list(struct pax_file_info *fi)
{
	const char *fn;

	if (fi->pathname && (*fi->pathname))
		fn = fi->pathname;
	else
		fn = _("(pathname missing)");

	if (!opt_verbose)
		printf("%s\n", fn);
	else {
		struct tm *tm;
		time_t t;
		char datebuf[128];

		if (fi->mtime > 0)
			t = fi->mtime;
		else
			t = time(NULL);
		tm = localtime(&t);
		my_strftime(datebuf, sizeof(datebuf), "%b %m %H:%M", tm);
		datebuf[sizeof(datebuf) - 1] = 0;

		printf("-%c%c%c%c%c%c%c%c%c %4u %4u %8" PRIuMAX " %s %s\n",
		       fi->mode & S_IRUSR ? 'r' : '-',
		       fi->mode & S_IWUSR ? 'w' : '-',
		       fi->mode & S_IXUSR ? 'x' : '-',
		       fi->mode & S_IRGRP ? 'r' : '-',
		       fi->mode & S_IWGRP ? 'w' : '-',
		       fi->mode & S_IXGRP ? 'x' : '-',
		       fi->mode & S_IROTH ? 'r' : '-',
		       fi->mode & S_IWOTH ? 'w' : '-',
		       fi->mode & S_IXOTH ? 'x' : '-',
		       (unsigned int) fi->uid,
		       (unsigned int) fi->gid,
		       fi->size,
		       datebuf,
		       fn);
	}

	return 0;
}

static int pax_read_archive(void)
{
	const char *pr_fn = _("(standard input)");
	int fd = STDIN_FILENO;
	int have_stdin = 1;
	int rc = 0;
	char *buf;

	if (opt_mode == PM_LIST) {
		pax_ops.file_start = pax_list;
		pax_ops.file_end = pax_noop_file_end;
		pax_ops.file_data = pax_noop_file_data;
	}

	if (opt_archive_fn) {
		fd = open(opt_archive_fn, O_RDONLY);
		if (fd < 0) {
			perror(opt_archive_fn);
			return 1;
		}
		pr_fn = opt_archive_fn;
		have_stdin = 0;
	}

	assert(block_size > 0);
	buf = (char *) xmalloc(block_size);

	pax_ops.input_init();

	while (1) {
		ssize_t rrc;
		int prc;

		rrc = read(fd, buf, block_size);
		if (rrc == 0)
			break;
		if (rrc < 0) {
			perror(pr_fn);
			rc = 1;
			goto out;
		}

		size_t buflen = rrc;
		prc = pax_ops.input(buf, &buflen);
		if (prc < 0) {
			fprintf(stderr, "input failed: err %d\n", prc);
			rc = 1;
			goto out;
		}
	}

	pax_ops.input_fini();

out:
	free(buf);
	if ((!have_stdin) && (close(fd) < 0))
		perror(pr_fn);
	return rc;
}

#define COPY(arg) do { fi.arg = lst->st_##arg; } while (0)

static int pax_file_arg(struct walker *w, const char *fn, const struct stat *lst)
{
	struct pax_file_info fi;
	int rc = 0;

	memset(&fi, 0, sizeof(fi));
	fi.pathname	= xstrdup(fn);
	fi.dev		= lst->st_dev;
	fi.inode	= lst->st_ino;
	COPY(mode);
	COPY(nlink);
	COPY(uid);
	COPY(gid);
	COPY(rdev);
	COPY(size);
	COPY(atime);
	COPY(mtime);
	COPY(ctime);

	if (S_ISLNK(lst->st_mode)) {
		char pathbuf[PATH_MAX + 1];
		if (readlink(fn, pathbuf, sizeof(pathbuf)) < 0) {
			perror(fn);
			w->exit_status = 1;
			goto out;
		}

		fi.linkname = xstrdup(pathbuf);
	}

	rc = pax_ops.file_start(&fi);
	if (rc)
		goto out;

	if (S_ISREG(lst->st_mode)) {
		char buf[4096];

		int fd = open(fn, O_RDONLY);
		if (fd < 0) {
			perror(fn);
			w->exit_status = 1;
			goto out;
		}

#ifdef POSIX_FADV_SEQUENTIAL
		posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

		while (1) {
			int frc;
			ssize_t rrc = read(fd, buf, sizeof(buf));
			if (rrc == 0)
				break;
			if (rrc < 0) {
				perror(fn);
				w->exit_status = 1;
				break;
			}

			frc = pax_ops.file_data(&fi, buf, rrc);
			if (frc) {
				w->exit_status = 1;
				break;
			}
		}

		if (close(fd) < 0)
			w->exit_status = 1;
	}

	rc = pax_ops.file_end(&fi);

	if ((rc == 0) && S_ISDIR(lst->st_mode) && opt_recurse)
		rc = RC_RECURSE;

out:
	pax_fi_clear(&fi);
	return rc;
}

#undef COPY

static void pax_init_formats(void)
{
	memset(&pax_ops, 0, sizeof(pax_ops));

	switch(opt_format) {
	case FMT_PAX:	pax_init_operations(&pax_ops);		break;
	case FMT_CPIO:	cpio_init_operations(&pax_ops);		break;
	case FMT_USTAR:	ustar_init_operations(&pax_ops);	break;
	case FMT_ZIP:	zip_init_operations(&pax_ops);		break;
	}
}

static int pax_pre_walk(struct walker *w)
{
	if ((!opt_read) && (!opt_write))
		opt_mode = PM_LIST;
	else if ((opt_read) && (!opt_write))
		opt_mode = PM_READ;
	else if ((!opt_read) && (opt_write))
		opt_mode = PM_WRITE;
	else
		opt_mode = PM_COPY;

	pax_init_formats();

	if ((opt_mode == PM_WRITE) || (opt_mode == PM_COPY)) {
		walker.cmdline_arg = pax_file_arg;
		walker.flags |= WF_STAT;
		if (opt_recurse)
			walker.flags |= WF_RECURSE;

		if (pax_ops.archive_start) {
			int rc = pax_ops.archive_start();
			if (rc)
				return rc;
		}
	}

	return 0;
}

static int pax_post_walk(struct walker *w)
{
	if ((opt_mode == PM_LIST) || (opt_mode == PM_READ))
		return pax_read_archive();

	if (pax_ops.archive_end)
		w->exit_status = pax_ops.archive_end();
	return w->exit_status;
}

int main (int argc, char *argv[])
{
	walker.argp			= &argp;
	walker.pre_walk			= pax_pre_walk;
	walker.post_walk		= pax_post_walk;
	walker.cmdline_arg		= pax_noop_arg;
	return walk(&walker, argc, argv);
}
