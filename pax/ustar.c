
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE			/* for strnlen(3) */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <libpu.h>
#include "pax.h"


enum random_ustar_constants {
	REC_SZ			= 512,
};

struct hdr_ustar {
	char us_name[100];
	char us_mode[8];
	char us_uid[8];
	char us_gid[8];
	char us_size[12];
	char us_mtime[12];
	char us_chksum[8];
	char us_typeflag;
	char us_linkname[100];
	char us_magic[6];
	char us_version[2];
	char us_uname[32];
	char us_gname[32];
	char us_devmajor[8];
	char us_devminor[8];
	char us_prefix[155];
};

struct ustar_state {
	bool			in_archive;
	bool			seen_first_block;
	unsigned int		data_blocks;
	unsigned int		data_bytes;

	struct pax_file_info	curfile;
};

static struct ustar_state global_state;
static const char zero_rec[REC_SZ] = "";


static void ustar_input_init(void)
{
	struct ustar_state *state = &global_state;

	memset(state, 0, sizeof(*state));
	state->in_archive = 1;
}

static void ustar_input_fini(void)
{
}

static int ustar_file_end(struct ustar_state *state)
{
	return pax_ops.file_end(&state->curfile);
}

static int ustar_hdr_cksum(const struct hdr_ustar *hdr)
{
	struct hdr_ustar *hdr_copy;
	unsigned char hdrbuf[REC_SZ];
	unsigned int sum = 0;
	long cksum;
	int i;

	/* copy to memory we can modify (fill cksum field with spaces) */
	memcpy(hdrbuf, hdr, REC_SZ);
	hdr_copy = (struct hdr_ustar *) hdrbuf;

	/* get cksum */
	cksum = octal_str(hdr->us_chksum, sizeof(hdr->us_chksum));
	if (cksum < 0)
		return 0;

	/* fill cksum field with spaces */
	memset(hdr_copy->us_chksum, ' ', sizeof(hdr->us_chksum));

	/* calculate sum */
	for (i = 0; i < REC_SZ; i++)
		sum += hdrbuf[i];

	/* check sum */
	return (cksum == sum);
}

static void ustar_hdr_str(const char *s_in, size_t maxlen, char **s_out)
{
	size_t len;
	char *s;

	len = strnlen(s_in, maxlen);
	s = xmalloc(len + 1);

	memcpy(s, s_in, len);
	s[len] = 0;

	*s_out = s;
}

#define COPYSTR(hdrname,finame) do {					\
	ustar_hdr_str(hdr->hdrname, sizeof(hdr->hdrname), &fi->finame);	\
	} while (0)
#define COPYOCTAL(hdrname,finame) do {					\
	if ((fi->finame = octal_str(hdr->hdrname, sizeof(hdr->hdrname))) < 0) \
		return PXE_GARBAGE;					\
	} while (0)

static int ustar_hdr_record(struct ustar_state *state, const char *buf)
{
	const struct hdr_ustar *hdr = (const struct hdr_ustar *) buf;
	struct pax_file_info *fi;
	char *basename = NULL, *dirname = NULL;

	if (!ustar_hdr_cksum(hdr))
		return PXE_GARBAGE;

	fi = &state->curfile;
	pax_fi_clear(fi);

	ustar_hdr_str(hdr->us_name, sizeof(hdr->us_name), &basename);
	COPYOCTAL(us_mode, mode);
	fi->mode &= ACCESSPERMS;
	COPYOCTAL(us_uid, uid);
	COPYOCTAL(us_gid, gid);
	COPYOCTAL(us_size, size);
	COPYOCTAL(us_mtime, mtime);

	switch (hdr->us_typeflag) {
	case '1':	fi->hardlink = true;	/* fall through */
	case '7':				/* fall through */
	case '0':	fi->mode |= S_IFREG; break;
	case '2':	fi->mode |= S_IFLNK; break;
	case '3':	fi->mode |= S_IFCHR; break;
	case '4':	fi->mode |= S_IFBLK; break;
	case '5':	fi->mode |= S_IFDIR; break;
	case '6':	fi->mode |= S_IFIFO; break;
	default:
		return PXE_GARBAGE;
	}

	COPYSTR(us_linkname, linkname);
	if (!strncmp("ustar", hdr->us_magic, sizeof(hdr->us_magic))) {
		COPYSTR(us_uname, username);
		COPYSTR(us_gname, groupname);
		COPYOCTAL(us_devmajor, dev_major);
		COPYOCTAL(us_devminor, dev_minor);
		ustar_hdr_str(hdr->us_prefix, sizeof(hdr->us_prefix), &dirname);
	} else {
		/* old Unix tar format */
		if (*basename) {
			size_t len = strlen(basename);
			if (basename[len - 1] == '/') {
				basename[len - 1] = 0;
				fi->mode &= ~S_IFREG;
				fi->mode |= S_IFDIR;
			}
		}
	}

	if (dirname && basename) {
		fi->pathname = strpathcat(dirname, basename);
		free(dirname);
		free(basename);
	} else if (basename)
		fi->pathname = basename;
	else if (dirname)
		free(dirname);

	if (S_ISREG(fi->mode)) {
		state->data_bytes = fi->size;
		state->data_blocks = (fi->size / REC_SZ);
		if (fi->size % REC_SZ)
			state->data_blocks++;
	} else {
		state->data_blocks = 0;
		state->data_bytes = 0;
	}

	return pax_ops.file_start(fi);
}

#undef COPYSTR
#undef COPYOCTAL

static int ustar_data_record(struct ustar_state *state, const char *buf)
{
	int rc;

	if (state->data_blocks > 1) {
		rc = pax_ops.file_data(&state->curfile, buf, REC_SZ);
		if (rc)
			return rc;

		state->data_blocks--;
		state->data_bytes -= REC_SZ;
	} else {
		rc = pax_ops.file_data(&state->curfile, buf, state->data_bytes);
		if (rc)
			return rc;

		state->data_blocks = 0;
		state->data_bytes = 0;
	}

	return 0;
}

static int ustar_input_record(struct ustar_state *state, const char *buf)
{
	/* if we're in the middle of reading a file's data,
	 * continue doing so
	 */
	if (state->data_blocks)
		return ustar_data_record(state, buf);

	/* otherwise, we're at a header or EOF record.  end the 
	 * previous file
	 */
	if (state->seen_first_block) {
		int rc = ustar_file_end(state);
		if (rc)
			return rc;
	}

	/* FIXME: detect two consecutive zero records as EOA */
	if (!memcmp(buf, zero_rec, REC_SZ)) {
		state->in_archive = false;
		return 0;
	}

	return ustar_hdr_record(state, buf);
}

static int ustar_input(const char *buf, size_t *buflen_io)
{
	struct ustar_state *state = &global_state;
	size_t buflen = *buflen_io;
	int rc = 0;

	while (state->in_archive && (buflen > 0)) {
		if (buflen < REC_SZ) {
			rc = PXE_TRUNCATED;
			goto out;
		}

		rc = ustar_input_record(state, buf);
		if (rc)
			goto out;

		buf += REC_SZ;
		buflen -= REC_SZ;
	}

out:
	*buflen_io = buflen;
	return rc;
}

void ustar_blksz_check(void)
{
	/* fill in default block size, if necessary */
	if (block_size == 0)
		block_size = 10240;

	if ((block_size % REC_SZ) != 0) {
		fprintf(stderr, _("Block size %u not a multiple of 512 octets\n"),
			block_size);
		exit(EXIT_FAILURE);
	}
}

void ustar_init_operations(struct pax_operations *ops)
{
	ops->input_init = ustar_input_init;
	ops->input_fini = ustar_input_fini;
	ops->input = ustar_input;

	ustar_blksz_check();
}

