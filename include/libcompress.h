
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

#ifndef __LIBCOMPRESS_H__
#define __LIBCOMPRESS_H__

#include <stdint.h>

enum {
	COMPRESS_MAGIC1			= 0x1F, /* first two bytes of any */
	COMPRESS_MAGIC2			= 0x9D, /* LZC (POSIX compress) file */

	COMPRESS_BLOCK_MODE		= 0x80,

	COMPRESS_DEFBITS	= 14,	/* def. bits of compression supported */
	COMPRESS_MAXBITS	= 16,	/* max. bits of compression supported */
	COMPRESS_MINBITS	= 9,	/* max. bits of compression supported */
};

struct compress_state;

struct compress {
	struct compress_state	*cstate;

	const unsigned char	*next_in;
	unsigned int		avail_in;
	uintmax_t		total_in;

	unsigned char		*next_out;
	unsigned int		avail_out;
	uintmax_t		total_out;

	unsigned char		block_mode;
	int			code_minbits;
	int			code_maxbits;

	void (*dict_add) (void *dict, const void *buf, int len, long val);
	long (*dict_lookup) (void *dict, const void *buf, int len);
	void (*dict_clear) (void *dict);

	void * (*dict_new) (void);
	void (*dict_free) (void *dict);
};


extern void compress_io(struct compress *comp);
extern int compress_init(struct compress *comp);
extern int compress_fini(struct compress *comp);


#endif /* __LIBCOMPRESS_H__ */
