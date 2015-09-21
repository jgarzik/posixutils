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

#include <stdlib.h>
#include <string.h>
#include <libpu.h>

void *xmalloc(size_t size)
{
	void *mem = malloc(size);
	if (!mem)
		die(_("out of memory"));
	return mem;
}

void *xrealloc(void *ptr, size_t size)
{
	void *mem = realloc(ptr, size);
	if (!mem)
		die(_("out of memory"));
	return mem;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *mem = calloc(nmemb, size);
	if (!mem)
		die(_("out of memory"));
	return mem;
}

char *xstrdup(const char *s)
{
	char *ret = strdup(s);
	if (!ret)
		die(_("out of memory"));
	return ret;
}

char *strpathcat(const char *dirn, const char *basen)
{
	char *s = (char *) xmalloc(strlen(dirn) + strlen(basen) + 2);
	sprintf(s, "%s/%s", dirn, basen);
	return s;
}

