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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <libpu.h>


int main (void)
{
	int rc;

	pu_init();

	char *s = getlogin();
	if (!s) {
		perror(_("getlogin(3)"));
		return 1;
	}

	rc = write_buf(s, strlen(s));
	rc |= write_buf("\n", 1);

	return rc;
}

