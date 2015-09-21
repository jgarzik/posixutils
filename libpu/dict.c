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

enum dictionary_constants {
	DE_BUF_SZ		= 64 * 1024, /* startup string table sz */
	DICT_SZ			= 9000, /* hash table size; just a guess */
};

struct dict_entry {
	struct dict_entry	*next;
	long			val;
	int			key_len;
	unsigned char		key[0];
};

struct dictionary {
	void			*de_buf;
	int			de_buf_len;
	int			de_buf_pos;

	struct dict_entry	*table[DICT_SZ];
};

struct dict_entry *new_de(void *opaque, int len)
{
	struct dictionary *d = (struct dictionary *) opaque;
	unsigned long addr;

	/*
	 * guarantee that the item we are allocating will
	 * fit in de->de_buf, 8-byte aligned.
	 * hopefully with this design, calls to realloc(3)
	 * are few and far between, making this an uncommon
	 * code path.
	 */
	if ((d->de_buf_len - d->de_buf_pos) < len) {
		if (d->de_buf_len == 0)
			d->de_buf_len = DE_BUF_SZ;
		else {
			d->de_buf_len *= 2;
			d->de_buf_len += len;
		}

		d->de_buf = xrealloc(d->de_buf, d->de_buf_len);
	}

	/* determine address for new dict_entry, and align it */
	addr = (unsigned long) d->de_buf;
	addr += d->de_buf_pos;

	/* decrease free space in de->de_buf */
	d->de_buf_pos += len;
	while ((d->de_buf_pos < d->de_buf_len) && (d->de_buf_pos & 0x7))
		d->de_buf_pos++;

	/* finally, return the address of the data area allocated */
	return (struct dict_entry *) (void *) addr;
}

/* hash function from Karl Nelson in a post to gtk-devel-list */
static inline unsigned int x31_hash (const void *buf, int len)
{
	const char *p = (const char *) buf;
	unsigned int h = 0;

	for (; len > 0; len--, p++)
 		h = ( h << 5 ) - h + *p;
 	return h;
}

void dict_add(void *opaque, const void *buf, int len, long val)
{
	struct dictionary *d = (struct dictionary *) opaque;
	struct dict_entry *de;
	unsigned int hash, idx;

	/* create new dictionary entry */
	de = new_de(d, sizeof(struct dict_entry) + len);
	de->val = val;
	de->key_len = len;
	memcpy(de->key, buf, len);

	/* hash string */
	hash = x31_hash(buf, len);
	idx = hash % DICT_SZ;

	/* add to dictionary */
	de->next = d->table[idx];
	d->table[idx] = de;
}

long dict_lookup(void *opaque, const void *buf, int len)
{
	struct dictionary *d = (struct dictionary *) opaque;
	struct dict_entry *de;
	unsigned int hash, idx;

	/* hash key */
	hash = x31_hash(buf, len);
	idx = hash % DICT_SZ;

	/* search for key in 'idx' hash table slot */
	de = d->table[idx];
	while (de) {
		if ((de->key_len == len) && (!memcmp(de->key, buf, len)))
			return de->val;

		de = de->next;
	}

	return -1;
}

void dict_clear(void *opaque)
{
	struct dictionary *d = (struct dictionary *) opaque;

	/* delete existing dictionary entries */
	memset(&d->table, 0, sizeof(d->table));
	if (d->de_buf)
		d->de_buf_pos = 0;
}

void dict_free(void *opaque)
{
	struct dictionary *d = (struct dictionary *) opaque;

	dict_clear(d);

	free(d->de_buf);
	d->de_buf = NULL;
	d->de_buf_len = 0;
	d->de_buf_pos = 0;

	free(d);
}

void dict_init(void *opaque)
{
	struct dictionary *d = (struct dictionary *) opaque;

	dict_clear(d);
}

void *dict_new(void)
{
	struct dictionary *d = (struct dictionary *) xcalloc(1, sizeof(struct dictionary));
	d->de_buf = xmalloc(DE_BUF_SZ);
	d->de_buf_len = DE_BUF_SZ;
	return d;
}
