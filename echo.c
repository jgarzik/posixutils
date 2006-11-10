/*
 * Copyright 2005-2006 Jeff Garzik <jgarzik@pobox.com>
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void escaped_string (const char *s)
{
	int ch, escape = 0;

	while (*s) {
		ch = *s;

		if (escape) {
			switch (ch) {
			case 'a':	putchar('\a'); break;
			case 'b':	putchar('\b'); break;
			case 'c':	exit(0);
			case 'f':	putchar('\f'); break;
			case 'n':	putchar('\n'); break;
			case 'r':	putchar('\r'); break;
			case 't':	putchar('\t'); break;
			case 'v':	putchar('\v'); break;
			case '\\':	putchar('\\'); break;
			default:	printf("\\%c", ch); break;
			}
			escape = 0;
		} else {
			if (ch == '\\')
				escape = 1;
			else
				putchar(ch);
		}

		s++;
	}
}

int main (int argc, char *argv[])
{
	int i;
	const char *arg;

	for (i = 1; i < argc; i++) {
		arg = argv[i];
		if (strchr(arg, '\\'))
			escaped_string(arg);
		else
			printf("%s", arg);

		if (i < (argc - 1))
			putchar(' ');
	}

	putchar('\n');
	return 0;
}
