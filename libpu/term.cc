
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
#include <curses.h>
#include <term.h>

static char default_term[16] = "vt100";

char *get_terminal(void)
{
	char *term = getenv("TERM");
	if (!term || !strlen(term))
		term = default_term;
	return term;
}

char *xtigetstr(const char *capname)
{
	char *s = tigetstr(capname);
	if (s == (char *)-1)
		s = NULL;

	return s;
}

