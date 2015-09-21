
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
#include <stdbool.h>
#include <stdio.h>
#include <argp.h>

#if ENABLE_NLS
# include <libintl.h>
# define _(String) gettext (String)
#else
# define _(String) String
#endif
#define N_(String) String


#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);    \
        _x < _y ? _x : _y; })

#define max(x,y) ({ \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);    \
        _x > _y ? _x : _y; })

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
	case ARGP_KEY_ARG:	slist_push(&walker.strlist, arg, false); break;
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

struct pathelem {
	char		*dirc;
	char		*dirn;
	char		*basec;
	char		*basen;
};

struct strmap {
	const char	*name;
	int		val;
};

struct strent {
	const char		*s;
	bool			alloced;
};

enum random_strlist_constants {
	STRLIST_STATIC		= 1024,
};

struct strlist {
	struct strent		buf[STRLIST_STATIC];
	struct strent		*list;
	unsigned int		alloc_len;
	unsigned int		len;
};

enum walker_flags {
	WF_NO_FILES_STDIN	= (1 << 0),
	WF_STAT			= (1 << 1),
	WF_FOLLOW_LINK		= (1 << 2),
	WF_FOLLOW_LINK_CMDLINE	= (1 << 3),
	WF_RECURSE		= (1 << 4),
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

	void			*priv;

	int			curdir_fd;
	struct strlist		strlist;
};

typedef int (*idir_actor_t)(int fd, const char *dirn, const char *basen);

extern ssize_t copy_fd(const char *dest_fn, int dest_fd,
		   const char *src_fn, int src_fd);
extern int __walk_cmdline(struct cmdline_walker *cw, int idx);
extern int walk_cmdline(struct cmdline_walker *cw);
extern int parse_cmdline(struct cmdline_walker *cw);
extern int write_buf(const void *, size_t);
extern int write_fd(int fd, const void *buf, size_t count, const char *fn);
extern void die(const char *msg);
extern error_t noopts_parse_opt (int key, char *arg, struct argp_state *state);
extern int ask_question(const char *prefix, const char *msg, const char *fn);

extern struct dict_entry *new_de(void *d, int len);
extern void dict_add(void *d, const void *buf, int len, long val);
extern long dict_lookup(void *d, const void *buf, int len);
extern void dict_clear(void *d);
extern void dict_free(void *d);
extern void dict_init(void *d);
extern void *dict_new(void);

extern const char file_args_doc[];
extern struct argp_option no_options[];
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern char *xstrdup(const char *s);
extern char *xgetcwd(void);
extern char *strpathcat(const char *dirn, const char *basen);
extern struct pathelem *path_split(const char *pathname);
extern void path_free(struct pathelem *pe);
extern int have_dots(const char *fn);

extern char *get_terminal(void);
extern char *xtigetstr(const char *capname);

extern void pu_init(void);
extern int ro_file_open(FILE **f_io, const char *fn);
extern int iterate_directory(int old_dirfd, const char *dirfn, const char *basen,
			      int opt_force, idir_actor_t actor);
extern int is_portable_char(int ch);
extern int map_lookup(const struct strmap *map, const char *key);
extern void slist_push(struct strlist *slist, const char *s, bool alloced);
extern void slist_free(struct strlist *slist);
extern char *slist_shift(struct strlist *slist);
extern char *slist_pop(struct strlist *slist);
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

static inline const char *slist_ref(struct strlist *slist, unsigned int idx)
{
	return slist->list[idx].s;
}

#ifdef HAVE___FSETLOCKING
#include <stdio_ext.h>
#else
enum { FSETLOCKING_BYCALLER = 0, };
static inline int __fsetlocking(FILE *stream, int type)
{
	return type;
}
#endif

#endif /* __POSIXUTILS_LIB_H__ */
