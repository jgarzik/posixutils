

/* Infix notation calculator--calc */

%{
#define YYSTYPE long long
#include <math.h>
#include <stdio.h>
extern int yylex();
extern void yyerror (const char *s);
%}

/* BISON Declarations */
%token TOK_NUM
%left '-' '+'
%left '*' '/'
%right '^'    /* exponentiation        */

/* Grammar follows */
%%
input:    /* empty string */
        | input line
;

line:   exp          { printf ("%lld\n", $1); }
;

exp:      TOK_NUM            { $$ = $1;         }
        | exp '+' exp        { $$ = $1 + $3;    }
        | exp '-' exp        { $$ = $1 - $3;    }
        | exp '*' exp        { $$ = $1 * $3;    }
        | exp '/' exp        { $$ = $1 / $3;    }
        | '(' exp ')'        { $$ = $2;         }
;
%%

