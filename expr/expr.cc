/*
 * Copyright 2015 Jeff Garzik <jgarzik@pobox.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <errno.h>
#include <libpu.h>

using namespace std;

enum token_type {
	TOK_EOF = 0,
	TOK_LPAREN = '(',	// (
	TOK_RPAREN = ')',	// )
	TOK_OR = '|',		// |
	TOK_AND = '&',		// &
	TOK_EQ = '=',		// =
	TOK_GT = '>',		// >
	TOK_GE,			// >=
	TOK_LT = '<',		// <
	TOK_LE,			// <=
	TOK_NE,			// !=
	TOK_ADD = '+',		// +
	TOK_SUB = '-',		// -
	TOK_MUL = '*',		// *
	TOK_DIV = '/',		// /
	TOK_MOD = '%',		// %
	TOK_COLON = ':',	// :
	TOK_STR,
};

extern "C" {
#if 0
long long yylval = 0;
#endif
#define YYSTYPE long long
#include "expr-parse.h"

extern int yyparse(void);
}

static vector<string> tokens;

static bool is_intstr(const string& s)
{
	if (s.size() == 0)
		return false;

	for (unsigned int i = 0; i < s.size(); i++) {
		int ch = s[i];
		if (!isdigit(ch)) {
			if ((i == 0) && (ch == '-'))
				/* ok */ ;
			else
				return false;
		}
	}

	return true;
}

static int parseToken(const string& s)
{
	if (s == "(") return TOK_LPAREN;
	else if (s == ")") return TOK_RPAREN;
	else if (s == "|") return TOK_OR;
	else if (s == "&") return TOK_AND;
	else if (s == "=") return TOK_EQ;
	else if (s == ">") return TOK_GT;
	else if (s == ">=") return TOK_GE;
	else if (s == "<") return TOK_LT;
	else if (s == "<=") return TOK_LE;
	else if (s == "!=") return TOK_NE;
	else if (s == "+") return TOK_ADD;
	else if (s == "-") return TOK_SUB;
	else if (s == "*") return TOK_MUL;
	else if (s == "/") return TOK_DIV;
	else if (s == "%") return TOK_MOD;
	else if (s == ":") return TOK_COLON;
	else if (is_intstr(s)) return TOK_NUM;
	else return TOK_STR;
}

extern "C" void yyerror (const char *s)
{
	fprintf (stderr, "%s\n", s);
}

extern "C" int yylex(void)
{
	if (tokens.size() == 0)
		return TOK_EOF;

	string s = tokens.front();
	tokens.erase(tokens.begin());

	int tt = parseToken(s);
	if (tt == TOK_NUM) {
		errno = 0;
		char *endptr = NULL;
		yylval = strtoll(s.c_str(), &endptr, 10);
		if (errno != 0)
			return -1;
	}

	return (int) tt;
}

int main (int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
		tokens.push_back(argv[i]);

	return yyparse();
}

