
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
#include "pax.h"


struct pax_state {
};

static struct pax_state state;

static void pax_input_init(void)
{
	memset(&state, 0, sizeof(state));
}

static void pax_input_fini(void)
{
}

static int pax_input(const char *buf, size_t *buflen)
{
	return -1; /* TODO */
}

static void pax_blksz_check(void)
{
	/* fill in default block size, if necessary */
	if (block_size == 0)
		block_size = 5120;
}

void pax_init_operations(struct pax_operations *ops)
{
	ops->input_init = pax_input_init;
	ops->input_fini = pax_input_fini;
	ops->input = pax_input;

	pax_blksz_check();
	ustar_blksz_check();
}

