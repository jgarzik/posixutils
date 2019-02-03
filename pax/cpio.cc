
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
#include <string.h>
#include <limits.h>
#include <libpu.h>
#include <assert.h>
#include "pax.h"


enum cpio_input_state {
	CS_HDR,
	CS_NAME,
	CS_DATA,
	CS_EOF,				/* end of file entry */
	CS_TRAILER,
	CS_EOA,				/* end of archive */
};

struct hdr_cpio {
	char				c_magic[6];
	char				c_dev[6];
	char				c_ino[6];
	char				c_mode[6];
	char				c_uid[6];
	char				c_gid[6];
	char				c_nlink[6];
	char				c_rdev[6];
	char				c_mtime[11];
	char				c_namesize[6];
	char				c_filesize[11];
};

struct cpio_state {
	struct pax_file_info		curfile;

	enum cpio_input_state		input_state;
	uintmax_t			input_bytes;

	struct hdr_cpio			input_hdr;
	unsigned int			input_hdr_pos;

	unsigned int			name_len;
	unsigned int			name_pos;

	uintmax_t			file_bytes;
};

static struct cpio_state global_state;


static void cpio_input_init(void)
{
	struct cpio_state *state = &global_state;

	memset(state, 0, sizeof(*state));

	state->input_state = CS_HDR;
}

static void cpio_input_fini(void)
{
	/* TODO */
}

#define COPYOCTAL(hdrname,finame) do {					      \
	if ((fi->finame = octal_str(hdr->hdrname, sizeof(hdr->hdrname))) < 0) \
		return PXE_GARBAGE;					      \
	} while (0)

static int cpio_hdr(struct cpio_state *state)
{
	struct hdr_cpio *hdr = &state->input_hdr;
	struct pax_file_info *fi;
	long l;

	if (memcmp(state->input_hdr.c_magic, "070707", 6))
		return PXE_GARBAGE;

	fi = &state->curfile;
	pax_fi_clear(fi);

	COPYOCTAL(c_dev, dev);
	COPYOCTAL(c_ino, inode);
	COPYOCTAL(c_mode, mode);
	COPYOCTAL(c_uid, uid);
	COPYOCTAL(c_gid, gid);
	COPYOCTAL(c_nlink, nlink);
	COPYOCTAL(c_rdev, rdev);
	COPYOCTAL(c_mtime, mtime);

	l = octal_str(state->input_hdr.c_namesize,
		      sizeof(state->input_hdr.c_namesize));
	if ((l < 0) || (l > PATH_MAX))
		return PXE_GARBAGE;
	state->name_len = l;

	fi->pathname.assign(state->name_len, 0);

	COPYOCTAL(c_filesize, size);
	state->file_bytes = fi->size;

	return 0;
}

#undef COPYOCTAL

static int cpio_hdr_name(struct cpio_state *state)
{
	struct pax_file_info *fi = &state->curfile;

	if (fi->pathname == "TRAILER!!!") {
		state->input_state = CS_EOA;
		return 0;
	}

	return pax_ops.file_start(&state->curfile);
}

#define SWALLOW(bytes) do {				\
	buf_in += (bytes);				\
	buflen -= (bytes);				\
	state->input_bytes += (bytes);			\
	} while (0)

static int cpio_input(const char *buf_in, size_t *buflen_io)
{
	struct cpio_state *state = &global_state;
	size_t buflen = *buflen_io;
	int rc = 0;

	while (buflen > 0) {
		switch (state->input_state) {
		case CS_HDR: {
			char *buf = (char *) &state->input_hdr;
			unsigned int i = std::min(buflen,
				sizeof(struct hdr_cpio) - state->input_hdr_pos);
			memcpy(buf + state->input_hdr_pos, buf_in, i);

			SWALLOW(i);
			state->input_hdr_pos += i;

			if (state->input_hdr_pos == sizeof(struct hdr_cpio)) {
				int trc = cpio_hdr(state);
				if (trc) {
					rc = trc;
					goto out;
				}

				state->input_hdr_pos = 0;
				state->input_state = CS_NAME;
			}
			break;
		}

		case CS_NAME: {
			assert(state->curfile.pathname.size() >= state->name_len);
			char *buf = &state->curfile.pathname[0];
			unsigned int i = state->name_len - state->name_pos;
			if (buflen < i)
				i = buflen;
			memcpy(buf + state->name_pos, buf_in, i);

			SWALLOW(i);
			state->name_pos += i;

			if (state->name_pos == state->name_len) {
				if (state->curfile.size > 0)
					state->input_state = CS_DATA;
				else
					state->input_state = CS_EOF;

				int trc = cpio_hdr_name(state);
				if (trc) {
					rc = trc;
					goto out;
				}

				state->name_pos = 0;
			}
			break;
		}

		case CS_DATA: {
			size_t data_len = buflen;
			int trc;

			if ((uintmax_t)buflen > state->file_bytes)
				data_len = state->file_bytes;

			trc = pax_ops.file_data(&state->curfile, buf_in, data_len);
			if (trc) {
				rc = trc;
				goto out;
			}

			SWALLOW(data_len);
			state->file_bytes -= data_len;

			if (state->file_bytes == 0)
				state->input_state = CS_EOF;
			break;
		}

		case CS_EOF: {
			int trc = pax_ops.file_end(&state->curfile);
			if (trc) {
				rc = trc;
				goto out;
			}

			state->input_state = CS_HDR;
			break;
		}

		case CS_TRAILER:
			/* end of file; do nothing else */
			state->input_state = CS_EOA;
			break;

		case CS_EOA:
			*buflen_io = 0;	/* bitbucket anything after trailer */
			return 0;
		}
	}

out:
	*buflen_io = buflen;
	return rc;
}

#undef SWALLOW

static void cpio_blksz_check(void)
{
	/* fill in default block size, if necessary */
	if (block_size == 0)
		block_size = 5120;
}

void cpio_init_operations(struct pax_operations *ops)
{
	ops->input_init = cpio_input_init;
	ops->input_fini = cpio_input_fini;
	ops->input = cpio_input;

	cpio_blksz_check();
}

