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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <libpu.h>
#include <libcompress.h>


enum {
	CODE_CLEAR		= 256,	/* clear-dictionary special code */
	CODE_FIRST		= 257,	/* first dictionary code (block mode) */
	CODE_FIRST_NOBLOCK	= 256,	/* first dict. code (not block mode) */

	HASH_SZ			= 50003,

	INPUT_CHKPT		= 10000,
};


struct compress_state {
	uint64_t		chkpt;		/* next checkpoint */
	uint64_t		b_ratio;

	int			next_code;
	int			last_code;
	int			n_bits;		/* code word length */

	bool			need_flush;
	bool			flushing;

	int			out_bitlen;

	unsigned char		out_bytebuf[COMPRESS_MAXBITS];
	int			out_bytelen;
	unsigned char		*out_byteptr;

	int			code;
	int			code_pair;

	int			tab_code[HASH_SZ];
	int			tab_hash[HASH_SZ];
};


static void compress_block_reset(struct compress *comp);
static void compress_output_code(struct compress *comp, int code);


static void compress_eof_flush(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;

	/* absorb current character at cs->out_byteptr */
	if (cs->out_bitlen < 8)
		cs->out_bytelen--;
	cs->need_flush = true;
}

static int compress_output(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;
	unsigned int waiting = cs->out_byteptr - cs->out_bytebuf;
	int dist;

	/* if user buffer full, return immediately with buffer-full retval */
	if (comp->avail_out == 0)
		return -1;		/* buffer full */

	if (!cs->need_flush)
		return 0;		/* nothing to do */

	/* output len is lesser of (user buffer len, amount to output) */
	dist = min(comp->avail_out, waiting);
	assert(dist > 0);

	/* output to user buffer */
	memcpy(comp->next_out, cs->out_bytebuf, dist);
	comp->next_out += dist;
	comp->avail_out -= dist;
	comp->total_out += dist;

	/* if we copied all pending output, reset our output buffer */
	if (dist == waiting) {
		cs->out_byteptr = cs->out_bytebuf;
		cs->out_bytelen = cs->n_bits;
		cs->need_flush = false;

		return 0; /* done */
	}

	/* otherwise, shift remaining bytes in output buffer down */
	else {
		int newlen;

		assert(dist > 0);
		assert(waiting > dist);
		newlen = waiting - dist;

		memmove(cs->out_bytebuf, cs->out_bytebuf + dist, newlen);

		cs->out_bytelen = sizeof(cs->out_bytebuf) - newlen;
		cs->out_byteptr = cs->out_bytebuf + newlen;

		return -1;		/* buffer full */
	}
}

static void compress_hash_clear(struct compress_state *cs)
{
	unsigned int i;

	/* separate loops is more friendly to memory streaming */
	for (i = 0; i < HASH_SZ; i++)
		cs->tab_hash[i] = -1;
}

static void compress_block_reset(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;

	cs->n_bits = comp->code_minbits;
	cs->next_code = comp->block_mode ? CODE_FIRST : CODE_FIRST_NOBLOCK;
	cs->last_code = (1 << cs->n_bits) - 1;

	compress_hash_clear(cs);
}

static void compress_flush(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;

	while (cs->out_bytelen > 0)
		compress_output_code(comp, 0);
}

static void compress_chkpt(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;
	uint64_t ratio;

	cs->chkpt = comp->total_in + INPUT_CHKPT;

	ratio = (comp->total_in << 8) / comp->total_out;

	/* if ratio improved, simply make a note of this */
	if (ratio >= cs->b_ratio)
		cs->b_ratio = ratio;

	/* otherwise, clear dictionary */
	else {
		compress_output_code(comp, CODE_CLEAR);

		/* output rest of segment (zeroes), because decompressor
		 * won't notice n_bits increase until next segment
		 */
		compress_flush(comp);

		compress_block_reset(comp);
	}
}

static int compress_dict_add(struct compress *comp, unsigned int hash)
{
	int need_more_bits;
	struct compress_state *cs = comp->cstate;

	/* special, uncommon "need hash" case */
	if (hash >= HASH_SZ)
		hash = cs->code_pair % HASH_SZ;

	if (cs->tab_hash[hash] >= 0)
		return -1;	/* hash slot occupied */

	/* if next_code is larger than the max allowed
	 * under the current n_bits, we need to increase n_bits
	 */
	need_more_bits = (cs->next_code > cs->last_code);

	/* however, we cannot increase n_bits beyond code_maxbits */
	if (need_more_bits && (cs->n_bits == comp->code_maxbits))
		return -2;	/* dictionary full */

	/* add (buf,len)->code mapping to dictionary */
	cs->tab_code[hash] = cs->next_code++;
	cs->tab_hash[hash] = cs->code_pair;

	/* increase n_bits if necessary */
	if (need_more_bits) {
		/* output rest of segment (zeroes), because decompressor
		 * won't notice n_bits increase until next segment
		 */
		compress_flush(comp);

		cs->n_bits++;
		cs->last_code = (1 << cs->n_bits) - 1;
	}

	return 0;
}

static const unsigned char lmask[9] =
	{0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static const unsigned char rmask[9] =
	{0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

static void compress_output_code(struct compress *comp, int code_in)
{
	struct compress_state *cs = comp->cstate;
	int n_bits = cs->n_bits;
	unsigned int shift;
	uint32_t code;
	unsigned char c;

	assert(code_in >= 0 && code_in <= 65535);
	assert(cs->out_bitlen > 0);
	assert(cs->out_bytelen > 0);
	assert(cs->need_flush == false);

	code = code_in;

	/* first output char */
	shift = 8 - cs->out_bitlen;
	c = (*cs->out_byteptr) & rmask[shift];
	c |= (code << shift) & lmask[shift];

	assert(cs->out_bytelen > 0);
	*cs->out_byteptr++ = c;
	cs->out_bytelen--;

	n_bits -= cs->out_bitlen;
	code >>= 8 - shift;

	/* second output char, if there are enough bits */
	if (n_bits >= 8) {
		c = code;

		assert(cs->out_bytelen > 0);
		*cs->out_byteptr++ = c;
		cs->out_bytelen--;

		n_bits -= 8;
		code >>= 8;
	}

	/* store the remaining bits */
	assert(n_bits < 8);
	cs->out_bitlen = 8 - n_bits;
	*cs->out_byteptr = code;

	if (cs->out_bytelen == 0)
		cs->need_flush = true;
}

void compress_io(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;
	unsigned char c;
	unsigned int hash = HASH_SZ + 1;

	if (cs->flushing) {
		cs->flushing = false;
		goto continue_output;
	}

	/* prime the pump */
	if (comp->total_in == 0) {
		c = *comp->next_in++;
		comp->avail_in--;
		comp->total_in++;

		cs->code = c;
	}

	while (comp->avail_in > 0) {
		int rc;

		/* flush output, if any */
		rc = compress_output(comp);
		if (rc < 0)
			return;

		/* C = next char in datastream */
		c = *comp->next_in++;
		comp->avail_in--;
		comp->total_in++;

		cs->code_pair = ((int)c << COMPRESS_MAXBITS) | cs->code;
		hash = cs->code_pair % HASH_SZ;

		/* if present in dictionary, P := P+C */
		if (cs->tab_hash[hash] == cs->code_pair)
			cs->code = cs->tab_code[hash];

		/* if not present in dictionary... */
		else {
			int ret;

			/* output code which denotes P to codestream */
			compress_output_code(comp, cs->code);

			cs->code = c;
continue_output:
			rc = compress_output(comp);
			if (rc < 0) {
				cs->flushing = true;
				return;
			}

			/* add string P+C to dictionary */
			ret = compress_dict_add(comp, hash);

			/* dictionary full */
			if (comp->block_mode && (ret < -1) &&
			    (!cs->need_flush) && (comp->total_in > cs->chkpt))
				compress_chkpt(comp);
		}
	}
}

int compress_init(struct compress *comp)
{
	struct compress_state *cs;

	cs = xcalloc(1, sizeof(*cs));
	comp->cstate = cs;

	compress_block_reset(comp);

	cs->code = -1;
	cs->chkpt = INPUT_CHKPT;

	assert(cs->n_bits <= sizeof(cs->out_bytebuf));
	cs->out_bytelen = cs->n_bits;
	cs->out_bitlen = 8;

	cs->out_byteptr = cs->out_bytebuf;

	return 0;
}

int compress_fini(struct compress *comp)
{
	struct compress_state *cs = comp->cstate;
	int ret;

	ret = compress_output(comp);
	if (ret < 0)
		return ret;	/* still more outputting to do */

	/* if p_code < 0, then there were zero bytes output,
	 * or we already flushed
	 */
	if (cs->code >= 0) {
		/* output code which denotes P to codestream */
		compress_output_code(comp, cs->code);
		cs->code = -1;

		/* flush out any non-zero bits */
		compress_eof_flush(comp);
	}

	ret = compress_output(comp);
	if (ret < 0)
		return ret;	/* still more outputting to do */

	free(cs);
	comp->cstate = NULL;

	return 0;
}

