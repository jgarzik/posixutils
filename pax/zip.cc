
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <libpu.h>
#include <zlib.h>
#include "pax.h"


enum random_zip_constants {
	OUTBUF_SZ			= (128 * 1024),

	ZFL_DATA_DESC			= (1 << 3), /* data desc. present */
};

enum zip_signatures {
	SIG_LOCAL_FILE			= 0x04034b50,
	SIG_EXTRA_DATA			= 0x08064b50,
	SIG_CENTRAL_DIR			= 0x02014b50,
	SIG_DIGI_SIG			= 0x05054b50,
	SIG_CENTRAL_DIR_END64		= 0x06064b50,
	SIG_DIR_EN64_LOCATOR		= 0x07064b50,
	SIG_CENTRAL_DIR_END		= 0x06054b50,
};

enum zip_extra_tags {
	TAG_UNIX			= 0x000D,
	TAG_UNIX_UIDGID			= 0x7855,
	TAG_UNIX_TIMES			= 0x5455,
};

enum zip_input_state {
	ZS_BEGIN_REC,
	ZS_HANDLE_REC,
	ZS_FILE_HDR,
	ZS_FILE_HDR_END,
	ZS_FILE_NAME,
	ZS_EXTRA,
	ZS_FLUSH_HDR,
	ZS_DATA,
	ZS_DATA_SKIP,
	ZS_DATA_DESC,
	ZS_EOR_FILE,
	ZS_EOR,
	ZS_CENTRAL_DIR,
	ZS_CENTRAL_DIR_END,
	ZS_END_CENTRAL,
	ZS_END_CENTRAL_END,
	ZS_SKIP_BYTES,
};

struct zip_local_file_hdr {
	uint16_t			min_version;
	uint16_t			flags;
	uint16_t			method;
	uint16_t			mtime;
	uint16_t			mdate;
	uint32_t			crc32;
	uint32_t			size_c;
	uint32_t			size_u;
	uint16_t			name_len;
	uint16_t			extra_len;
} __attribute__((packed));

struct zip_data_desc {
	uint32_t			crc32;
	uint32_t			size_c;
	uint32_t			size_u;
} __attribute__((packed));

struct zip_central_dir_hdr {
	uint16_t			creator_version;
	uint16_t			min_version;
	uint16_t			flags;
	uint16_t			method;
	uint16_t			mtime;
	uint16_t			mdate;
	uint32_t			crc32;
	uint32_t			size_c;
	uint32_t			size_u;
	uint16_t			name_len;
	uint16_t			extra_len;
	uint16_t			comment_len;
	uint16_t			disk_num_start;
	uint16_t			file_attr_int;
	uint32_t			file_attr_ext;
	uint32_t			local_hdr_offset;
} __attribute__((packed));

struct zip_end_central_dir_hdr {
	uint16_t			disk_num;
	uint16_t			dir_disk;
	uint16_t			disk_entries;
	uint16_t			total_entries;
	uint32_t			dir_size;
	uint32_t			dir_offset;
	uint16_t			comment_len;
} __attribute__((packed));

struct zip_extra_hdr {
	uint16_t			tag;
	uint16_t			len;
} __attribute__((packed));

struct zip_extra_unix {
	uint32_t			atime;
	uint32_t			mtime;
	uint16_t			uid;
	uint16_t			gid;
} __attribute__((packed));

struct zip_extra_unix_uidgid {
	uint16_t			uid;
	uint16_t			gid;
} __attribute__((packed));

struct zip_extra_unix_times {
	uint8_t				flags;
	uint32_t			mtime;
	uint32_t			atime;
	uint32_t			ctime;
} __attribute__((packed));

struct zip_state {
	enum zip_input_state		input_state;

	struct pax_file_info		curfile;

	uint32_t			signature;

	struct zip_local_file_hdr	input_hdr;
	struct zip_data_desc		input_data_desc;
	char				*extra;

	unsigned long			crc;

	struct zip_central_dir_hdr	central_dir_hdr;
	struct zip_end_central_dir_hdr	end_central_dir_hdr;

	char				*outbuf;

	unsigned int			data_bytes;

	char				*sch_buf;
	unsigned int			sch_buf_len;
	unsigned int			sch_buf_pos;
	enum zip_input_state		sch_next_state;

	z_stream			zs;
};

static struct zip_state global_state;


static void zip_sched_buf(struct zip_state *state, void *buf, size_t buflen,
			  enum zip_input_state next_state)
{
	state->sch_buf = (char *) buf;
	state->sch_buf_len = buflen;
	state->sch_buf_pos = 0;
	state->sch_next_state = next_state;
}

static size_t zip_sched_do(struct zip_state *state, const char *buf_in,
			   size_t buflen_in)
{
	size_t dist = state->sch_buf_len - state->sch_buf_pos;
	if (buflen_in < dist)
		dist = buflen_in;

	if (state->sch_buf)
		memcpy(state->sch_buf + state->sch_buf_pos, buf_in, dist);
	state->sch_buf_pos += dist;

	if (state->sch_buf_pos == state->sch_buf_len) {
		state->input_state = state->sch_next_state;
		state->sch_buf = NULL;
		state->sch_buf_len = 0;
		state->sch_buf_pos = 0;
	}

	return dist;
}

static void zip_input_init(void)
{
	struct zip_state *state = &global_state;
	memset(state, 0, sizeof(*state));

	state->input_state = ZS_BEGIN_REC;

	state->outbuf = (char *) xmalloc(OUTBUF_SZ);
}

static void zip_input_fini(void)
{
	struct zip_state *state = &global_state;

	free(state->outbuf);
}

#undef X2
#undef X4
#define X2(val) do { hdr->(val) = swab16(hdr->(val)); } while (0)
#define X4(val) do { hdr->(val) = swab32(hdr->(val)); } while (0)

static inline void zip_swap_lfh(struct zip_local_file_hdr *hdr)
{
#ifdef WORDS_BIGENDIAN
	X4(signature);
	X2(min_version);
	X2(flags);
	X2(method);
	X2(mtime);
	X2(mdate);
	X4(crc32);
	X4(size_c);
	X4(size_u);
	X2(name_len);
	X2(extra_len);
#endif
}

static inline void zip_swap_cfh(struct zip_central_dir_hdr *hdr)
{
#ifdef WORDS_BIGENDIAN
	X2(creator_version);
	X2(min_version);
	X2(flags);
	X2(method);
	X2(mtime);
	X2(mdate);
	X4(crc32);
	X4(size_c);
	X4(size_u);
	X2(name_len);
	X2(extra_len);
	X2(comment_len);
	X2(disk_num_start);
	X2(file_attr_int);
	X4(file_attr_ext);
	X4(local_hdr_offset);
#endif
}

static inline void zip_swap_ecdh(struct zip_end_central_dir_hdr *hdr)
{
#ifdef WORDS_BIGENDIAN
	X2(disk_num);
	X2(dir_disk);
	X2(disk_entries);
	X2(total_entries);
	X4(dir_size);
	X4(dir_offset);
	X2(comment_len);
#endif
}

#undef X2
#undef X4

static time_t dos_time_xlat(uint16_t dos_date, uint16_t dos_time)
{
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_sec	= (dos_time & 0x1f);
	tm.tm_min	= ((dos_time >> 5) & 0x3f);
	tm.tm_hour	= ((dos_time >> 11) & 0x1f);

	tm.tm_mday	= (dos_date & 0x1f);
	tm.tm_mon	= ((dos_date >> 5) & 0xf);
	tm.tm_year	= 1980 + ((dos_date >> 9) & 0x7f);

	return mktime(&tm);
}

static int zip_file_hdr_end(struct zip_state *state)
{
	struct zip_local_file_hdr *hdr = &state->input_hdr;
	struct pax_file_info *fi = &state->curfile;

	free(state->extra);

	pax_fi_clear(fi);

	zip_swap_lfh(hdr);

	fi->size		= hdr->size_u;
	fi->mtime		= dos_time_xlat(hdr->mdate, hdr->mtime);
	fi->compressed_size	= hdr->size_c;
	if (hdr->name_len > 0) {
		fi->pathname	= (char *) xmalloc(hdr->name_len + 1);
		memset(fi->pathname, 0, hdr->name_len + 1);
	}
	if (hdr->extra_len > 0)
		state->extra	= (char *) xmalloc(hdr->extra_len);

	state->data_bytes = fi->compressed_size;

	/* filter out unsupported compression methods */
	switch(hdr->method) {
	case 0:
		state->data_bytes = hdr->size_u;
		break;
	case 8:
		break;
	default:
		fprintf(stderr, "unsupported compression method %u\n",
			(unsigned int) hdr->method);
		return PXE_MISC_ERR;
	}

	state->input_state = ZS_FILE_NAME;
	state->crc = crc32(0, NULL, 0);

	return 0;
}

static int zip_flush_hdr(struct zip_state *state)
{
	size_t entry_len;
	ssize_t extra_len = state->input_hdr.extra_len;
	char *buf = state->extra;
	struct zip_extra_hdr ex_hdr;
	struct pax_file_info *fi = &state->curfile;

	while (extra_len >= sizeof(struct zip_extra_hdr)) {
		memcpy(&ex_hdr, buf, sizeof(ex_hdr));
		buf += sizeof(ex_hdr);
		extra_len -= sizeof(ex_hdr);

		entry_len = from_le16(ex_hdr.len);
		if (entry_len > extra_len)
			return PXE_GARBAGE;

		switch(from_le16(ex_hdr.tag)) {
		case TAG_UNIX: {
			struct zip_extra_unix *uhdr;
			if (entry_len < sizeof(struct zip_extra_unix))
				break;
			uhdr = (struct zip_extra_unix *) buf;
			fi->mtime = from_le32(uhdr->mtime);
			fi->uid = from_le16(uhdr->uid);
			fi->gid = from_le16(uhdr->gid);
			break;
		}

		case TAG_UNIX_UIDGID: {
			struct zip_extra_unix_uidgid *uhdr;
			if (entry_len < sizeof(struct zip_extra_unix_uidgid))
				break;
			uhdr = (struct zip_extra_unix_uidgid *) buf;
			fi->uid = from_le16(uhdr->uid);
			fi->gid = from_le16(uhdr->gid);
			break;
		}

		case TAG_UNIX_TIMES: {
			struct zip_extra_unix_times *uhdr;
			if (entry_len < sizeof(struct zip_extra_unix_times))
				break;
			uhdr = (struct zip_extra_unix_times *) buf;
			if (uhdr->flags & (1 << 0))
				fi->mtime = from_le32(uhdr->mtime);
			break;
		}

		default:
			break;
		}

		extra_len -= entry_len;
		buf += entry_len;
	}

	int rc = pax_ops.file_start(&state->curfile);
	if (rc)
		return rc;

	switch (state->input_hdr.method) {
	case 8:
		memset(&state->zs, 0, sizeof(state->zs));
		if (inflateInit2(&state->zs, -MAX_WBITS) != Z_OK)
			return PXE_MISC_ERR;
		break;
	default:
		/* do nothing */
		break;
	}

	return 0;
}

static int zip_stored_data(struct zip_state *state, const char **buf_io,
			   size_t *buflen_io)
{
	const char *buf_in = *buf_io;
	size_t buflen = *buflen_io;
	int trc;

	if (buflen > state->data_bytes)
		buflen = state->data_bytes;

	state->crc = crc32(state->crc, (const unsigned char *) buf_in, buflen);

	trc = pax_ops.file_data(&state->curfile, buf_in, buflen);
	if (trc)
		return trc;

	*buf_io += buflen;
	*buflen_io -= buflen;
	state->data_bytes -= buflen;

	if (state->data_bytes == 0) {
		if (state->input_hdr.crc32 != state->crc) {
			fprintf(stderr, _("crc32 mismatch\n"));
			return PXE_GARBAGE;
		}
		state->input_state = ZS_DATA_SKIP;
	}

	return 0;
}

static int zip_inflate_data(struct zip_state *state, const char **buf_io,
			    size_t *buflen_io)
{
	int rc = 0;
	int zrc, trc;
	size_t zlen;
	size_t buflen = *buflen_io;
	const char *buf_in = *buf_io;
	unsigned int pre_avail_in, pre_avail_out;

	/* grrrr: zlib doesn't know about 'const' */
	state->zs.next_in = (Bytef *) buf_in;
	state->zs.avail_in = buflen;
	if (buflen > state->data_bytes)
		state->zs.avail_in = state->data_bytes;

inflate_output:
	state->zs.next_out = (unsigned char *) state->outbuf;
	state->zs.avail_out = OUTBUF_SZ;

	pre_avail_in = state->zs.avail_in;
	pre_avail_out = state->zs.avail_out;
	zrc = inflate(&state->zs, Z_PARTIAL_FLUSH);

	/* handle consumed input bytes */
	zlen = pre_avail_in - state->zs.avail_in;
	if (zlen > 0) {
		buf_in += zlen;
		buflen -= zlen;
		assert(state->data_bytes >= zlen);
		state->data_bytes -= zlen;
	}

	/* handle output */
	zlen = pre_avail_out - state->zs.avail_out;
	if (zlen > 0) {
		state->crc = crc32(state->crc,
				   (const unsigned char *) state->outbuf, zlen);
		trc = pax_ops.file_data(&state->curfile,
				state->outbuf, zlen);
		if (trc) {
			rc = trc;
			goto out;
		}
	}

	/* end of stream */
	if (zrc == Z_STREAM_END) {
		zrc = inflateEnd(&state->zs);
		if (zrc != Z_OK) {
			rc = PXE_MISC_ERR;
			goto out;
		}

		if (state->input_hdr.crc32 != state->crc) {
			fprintf(stderr, _("crc32 mismatch\n"));
			rc = PXE_GARBAGE;
			goto out;
		}

		state->input_state = ZS_DATA_SKIP;
		goto out;
	}

	/* error */
	else if (zrc != Z_OK) {
		fprintf(stderr, "inflate failed: %d\n", zrc);
		rc = PXE_MISC_ERR;
		goto out;
	}

	if (state->zs.avail_in > 0)
		goto inflate_output;

out:
	*buf_io = buf_in;
	*buflen_io = buflen;
	return rc;
}

#define SWALLOW(bytes) do {				\
	buf_in += (bytes);				\
	buflen -= (bytes);				\
	} while (0)

static int zip_input(const char *buf_in, size_t *buflen_io)
{
	struct zip_state *state = &global_state;
	size_t buflen = *buflen_io;
	int trc, rc = 0;
	size_t count;

	while (buflen > 0) {
		switch (state->input_state) {
		case ZS_BEGIN_REC:
			if (!state->sch_buf)
				zip_sched_buf(state, &state->signature,
					      sizeof(state->signature),
					      ZS_HANDLE_REC);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_HANDLE_REC:
			switch (from_le32(state->signature)) {
			case SIG_LOCAL_FILE:
				state->input_state = ZS_FILE_HDR;
				break;
			case SIG_CENTRAL_DIR:
				state->input_state = ZS_CENTRAL_DIR;
				break;
			case SIG_CENTRAL_DIR_END:
				state->input_state = ZS_END_CENTRAL;
				break;
			default:
				fprintf(stderr, "unsupported SIG (garbage?): 0x%x\n",
					from_le32(state->signature));
				rc = PXE_GARBAGE;
				goto out;
			}
			break;

		case ZS_FILE_HDR:
			if (!state->sch_buf)
				zip_sched_buf(state, &state->input_hdr,
					      sizeof(state->input_hdr),
					      ZS_FILE_HDR_END);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_FILE_HDR_END:
			trc = zip_file_hdr_end(state);
			if (trc) {
				rc = trc;
				goto out;
			}
			break;

		case ZS_FILE_NAME:
			if (!state->sch_buf)
				zip_sched_buf(state, state->curfile.pathname,
					      state->input_hdr.name_len,
					      ZS_EXTRA);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_EXTRA:
			if (!state->sch_buf)
				zip_sched_buf(state, state->extra,
					      state->input_hdr.extra_len,
					      ZS_FLUSH_HDR);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_FLUSH_HDR:
			trc = zip_flush_hdr(state);
			if (trc) {
				rc = trc;
				goto out;
			}

			state->input_state = ZS_DATA;
			break;

		case ZS_DATA:
			switch (state->input_hdr.method) {
			case 0: trc = zip_stored_data(state, &buf_in, &buflen);
				break;
			case 8: trc = zip_inflate_data(state, &buf_in, &buflen);
				break;
			default:
				trc = PXE_GARBAGE;
				break;
			}
			if (trc < 0) {
				rc = trc;
				goto out;
			}
			break;

		case ZS_DATA_SKIP:
			if (state->data_bytes > 0) {
				SWALLOW(1);
			} else {
				if (state->input_hdr.flags & ZFL_DATA_DESC)
					state->input_state = ZS_DATA_DESC;
				else
					state->input_state = ZS_EOR_FILE;
			}
			break;

		case ZS_DATA_DESC:
			if (!state->sch_buf)
				zip_sched_buf(state, &state->input_data_desc,
					      sizeof(state->input_data_desc),
					      ZS_EOR_FILE);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_EOR_FILE:
			trc = pax_ops.file_end(&state->curfile);
			if (trc) {
				rc = trc;
				goto out;
			}
			/* fall through */

		case ZS_EOR:
			state->input_state = ZS_BEGIN_REC;
			break;

		case ZS_CENTRAL_DIR:
			if (!state->sch_buf)
				zip_sched_buf(state, &state->central_dir_hdr,
					      sizeof(state->central_dir_hdr),
					      ZS_CENTRAL_DIR_END);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_CENTRAL_DIR_END: {
			struct zip_central_dir_hdr *hdr;

			hdr = &state->central_dir_hdr;
			zip_swap_cfh(hdr);

			count = hdr->name_len + hdr->extra_len +
				hdr->comment_len;

			zip_sched_buf(state, NULL, count, ZS_EOR);
			state->input_state = ZS_SKIP_BYTES;
			break;
		}

		case ZS_END_CENTRAL:
			if (!state->sch_buf)
				zip_sched_buf(state, &state->end_central_dir_hdr,
					      sizeof(state->end_central_dir_hdr),
					      ZS_END_CENTRAL_END);
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;

		case ZS_END_CENTRAL_END: {
			struct zip_end_central_dir_hdr *hdr;

			hdr = &state->end_central_dir_hdr;
			zip_swap_ecdh(hdr);

			count = hdr->comment_len;

			zip_sched_buf(state, NULL, count, ZS_EOR);
			state->input_state = ZS_SKIP_BYTES;
			break;
		}

		case ZS_SKIP_BYTES:
			count = zip_sched_do(state, buf_in, buflen);
			SWALLOW(count);
			break;
		}
	}

out:
	*buflen_io = buflen;
	return rc;
}

#undef SWALLOW

void zip_blksz_check(void)
{
	/* fill in default block size, if necessary */
	if (block_size == 0)
		block_size = 5120;
}

void zip_init_operations(struct pax_operations *ops)
{
	ops->input_init = zip_input_init;
	ops->input_fini = zip_input_fini;
	ops->input = zip_input;

	zip_blksz_check();
}

