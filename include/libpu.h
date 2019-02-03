
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

#ifndef __POSIXUTILS_LIB_H__
#define __POSIXUTILS_LIB_H__

#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <argp.h>
#include <dirent.h>
#include <regex.h>

#if ENABLE_NLS
# include <libintl.h>
# define _(String) gettext (String)
#else
# define _(String) String
#endif
#define N_(String) String


#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define STDIN_NAME "(stdin)"
#define STDOUT_NAME "(stdout)"
#define STDERR_NAME "(stderr)"

#define PU_OPT_BEGIN \
	switch (key) {
#define PU_OPT_END	\
	}		\
	return 0;
#define PU_OPT_IGNORE(char) \
	case (char):	/* do nothing */ break;
#define PU_OPT_ARG \
	case ARGP_KEY_ARG:	walker.arglist.push_back(arg); break;
#define PU_OPT_DEFAULT \
	default:	return ARGP_ERR_UNKNOWN;
#define PU_OPT_SET(char,var) \
	case (char): opt_##var = true; break;

#define DECLARE_PU_PARSE_ARGS \
static error_t args_parse_opt (int key, char *arg, struct argp_state *state) \
{				\
	PU_OPT_BEGIN		\
	PU_OPT_ARG		\
	PU_OPT_DEFAULT		\
	PU_OPT_END		\
}


enum cmdline_flags {
	CFL_STDIN_DASH		= (1 << 0),
	CFL_SKIP_LAST		= (1 << 1),
	CFL_NO_FILES_STDIN	= (1 << 2),
};

enum cmdline_errors {
	CERR_NONE,		/* no error */
	CERR_SYS,		/* errno-based error */
	CERR_NO_ARGS,		/* no file arguments specified */
};

struct cmdline_walker {
	int argc;
	char **argv;
	const struct argp *argp;
	unsigned long flags;
	int (*fn_actor)(struct cmdline_walker *cw, const char *fn);
	int (*fd_actor)(struct cmdline_walker *cw, const char *fn, int fd);
};

class pathelem {
public:
	std::string	dirn;
	std::string	basen;
};

struct strmap {
	const char	*name;
	int		val;
};

class Regex {
private:
	regex_t reg;
	std::string regex;
	int cflags;
	bool initd;

public:
	Regex() : initd(false) {}
	Regex(const std::string& regex_, int cflags_ = 0) : initd(false) {
		compile(regex_, cflags_);
	}
	Regex(const Regex& obj_copy) : initd(false) {
		compile(obj_copy.regex, obj_copy.cflags);
	}
	~Regex() {
		clear();
	}

	bool ok() { return initd; }
	void clear() {
		if (initd) {
			regfree(&reg);
			initd = false;
		}
	}

	bool compile(const std::string& regex_, int cflags_ = 0) {
		clear();

		int rc = regcomp(&reg, regex_.c_str(), cflags_);
		if (rc)
			return false;

		regex = regex_;
		cflags = cflags_;
		initd = true;
		return true;
	}
	bool match(const std::string& haystack, int eflags = 0) {
		if (!initd)
			return false;
		return (regexec(&reg, haystack.c_str(), 0, NULL, eflags) == 0);
	}
	bool match(const std::string& haystack, size_t nmatch, regmatch_t *matches, int eflags = 0) {
		if (!initd)
			return false;
		return (regexec(&reg, haystack.c_str(), nmatch, matches, eflags) == 0);
	}
	bool match(const std::string& haystack, std::string& out1, int eflags = 0);
	bool match(const std::string& haystack, std::string& out1, std::string& out2,
		   int eflags = 0);
	bool match(const std::string& haystack,
		   std::string& out1, std::string& out2, std::string& out3, int eflags = 0);
};

enum walker_flags {
	WF_NO_FILES_STDIN	= (1 << 0),
	WF_STAT			= (1 << 1),
	WF_FOLLOW_LINK		= (1 << 2),
	WF_FOLLOW_LINK_CMDLINE	= (1 << 3),
	WF_RECURSE		= (1 << 4),
	WF_STDIN_DASH		= (1 << 5),
	WF_NO_CLOSE		= (1 << 6),
};

enum walker_cmdline_arg_ret {
	RC_OK			= 0,
	RC_STOP_WALK		= -1,
	RC_RECURSE		= -2,
};

struct walker {
	uint32_t		flags;
	const struct argp	*argp;

	int (*init) (struct walker *w, int argc, char **argv);
	int (*pre_walk) (struct walker *w);
	int (*post_walk) (struct walker *w);

	int (*cmdline_arg) (struct walker *w, const char *fn,
			    const struct stat *st);
	int (*cmdline_fd) (struct walker *w, const char *fn, int fd);
	int (*dirent) (struct walker *w, const char *dirn, const char *basen,
		       const struct stat *st);
	int (*cmdline_file) (struct walker *w, const char *pr_fn,
			     FILE *f);

	int			exit_status;

	int			curdir_fd;
	std::vector<std::string> arglist;
};

typedef int (*idir_actor_t)(int fd, const char *dirn, const char *basen);

extern ssize_t copy_fd(const std::string& dest_fn, int dest_fd,
		const std::string& src_fn, int src_fd);
extern int __walk_cmdline(struct cmdline_walker *cw, int idx);
extern int walk_cmdline(struct cmdline_walker *cw);
extern int parse_cmdline(struct cmdline_walker *cw);
extern int write_buf(const void *, size_t);
extern int write_fd(int fd, const void *buf_, size_t count, const std::string& fn);
extern error_t noopts_parse_opt (int key, char *arg, struct argp_state *state);
extern bool ask_question(const std::string& prefix, const std::string& msg, const std::string& fn);

extern const char file_args_doc[];
extern struct argp_option no_options[];
extern bool path_split(const std::string& pathname, pathelem& pe);
extern bool have_dots(const std::string& fn);
extern void strsplit(const std::string& s,
			   std::vector<std::string>& sv);
extern void strsplit(const std::string& s, int delim,
			   std::vector<std::string>& sv);
extern void strsplit(const std::string& s, const std::string& regex,
			   std::vector<std::string>& sv);
extern void strbisect(const std::string& s, int delim,
		      std::string& s1, std::string& s2);

static inline std::string strpathcat(const std::string& dn, const std::string& bn)
{
	return dn + "/" + bn;
}

extern char *get_terminal(void);
extern char *xtigetstr(const char *capname);

extern void pu_init(void);
extern int ro_file_open(FILE **f_io, const char *fn);
extern int iterate_directory(int old_dirfd, const char *dirfn, const char *basen,
			      int opt_force, idir_actor_t actor);
extern int is_portable_char(int ch);
extern int map_lookup(const struct strmap *map, const char *key);
extern int walk(struct walker *w, int argc, char **argv);


static inline uint32_t swab32(uint32_t val)
{
	return	((val & 0x000000ffUL) << 24) |
		((val & 0x0000ff00UL) <<  8) |
		((val & 0x00ff0000UL) >>  8) |
		((val & 0xff000000UL) >> 24) ;
}

static inline uint16_t swab16(uint16_t val)
{
	return  ((val & 0x00ffU) << 8) |
		((val & 0xff00U) >> 8) ;

}

#ifdef WORDS_BIGENDIAN
#define from_le32(x) swab32(x)
#define from_le16(x) swab16(x)
#else
#define from_le32(x) (x)
#define from_le16(x) (x)
#endif

#ifdef HAVE___FSETLOCKING
#include <stdio_ext.h>
#else
enum { FSETLOCKING_BYCALLER = 0, };
static inline int __fsetlocking(FILE *stream, int type)
{
	return type;
}
#endif

#ifndef HAVE_FGETS_UNLOCKED
static inline char *fgets_unlocked(char *s, int size, FILE *stream)
{
	return fgets(s, size, stream);
}
#endif

#ifndef NAME_MAX
#define NAME_MAX MAXNAMLEN
#endif

#ifndef ACCESSPERMS
# define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#endif

#endif /* __POSIXUTILS_LIB_H__ */
