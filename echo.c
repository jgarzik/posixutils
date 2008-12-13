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

static void escaped_string(const char *);

int convert_octal(int num, int rem, int m)
{
	int q, r;

	q = num / 10;
	if (q > 0) {
		r += num % 10 * m;
		r += rem;
		m *= 8;
		convert_octal(q, r, m);
	} else
		return rem += num % 10 * m;
}

void get_octal(const char *s)
{
	int remainder = 0;
	int multiplier = 1;
	int oct;
	int olen;
	int slen;
	int i;

/* we except only 8-bit octal */
	const int maxlen = 4;
	const int upper_limit = 377;
	char o[maxlen];
	char octal_numbers[] = "01234567";

	olen = strspn(s, octal_numbers);
	slen = strlen(s);

	for (i = 0; i < maxlen && i < olen; i++)
		o[i] = s[i];
	o[i] = '\0';
	oct = atoi(o);
	if (oct > upper_limit) {
		printf("%s", s);
		return;
	}
	printf("%d", convert_octal(oct, remainder, multiplier));
	if (*(s+i) == '\0')
		return;		/* no reason to fiddle with the null byte */
	escaped_string(s + i);
	return;
}

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
			case '0':	get_octal(s); return;
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
