
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

#include <string.h>
#include <stdlib.h>
#include <libpu.h>

void slist_push(struct strlist *slist, const char *s, bool alloced)
{
	struct strent *ent;

	/* if uninitialized, point to static buffer */
	if (slist->list == NULL) {
		slist->list = slist->buf;
		slist->alloc_len = STRLIST_STATIC;
	}

	/* grow string pointer array if necessary */
	if (slist->len == slist->alloc_len) {
		size_t alloc_len;

		slist->alloc_len <<= 1;
		if (slist->alloc_len == 0)
			die(_("string list too large"));

		alloc_len = slist->alloc_len * sizeof(struct strent);

		if (slist->list == slist->buf) {
			slist->list = xmalloc(alloc_len);
			memcpy(slist->list, slist->buf, sizeof(slist->buf));
		} else
			slist->list = xrealloc(slist->list, alloc_len);
	}

	/* store string */
	ent = &slist->list[slist->len];
	ent->s = s;
	ent->alloced = alloced;
	slist->len++;
}

void slist_free(struct strlist *slist)
{
	unsigned int i;

	for (i = 0; i < slist->len; i++) {
		struct strent *ent = &slist->list[i];
		if (ent->alloced) {
			void *mem = (void *) ent->s;
			free(mem);
			ent->alloced = false;
		}
	}

	if (slist->list != slist->buf)
		free(slist->list);
	memset(slist, 0, sizeof(*slist));
}

char *slist_shift(struct strlist *slist)
{
	char *s;

	if (slist->len == 0)
		return NULL;

	s = (char *) slist->list[0].s;
	if (!slist->list[0].alloced)
		s = xstrdup(s);

	memmove(slist->list, slist->list + 1,
		(slist->len - 1) * sizeof(struct strent));

	slist->len--;

	return s;
}

char *slist_pop(struct strlist *slist)
{
	char *s;

	if (slist->len == 0)
		return NULL;

	s = (char *) slist->list[slist->len - 1].s;
	if (!slist->list[slist->len - 1].alloced)
		s = xstrdup(s);

	slist->len--;

	return s;
}

