
%{
#include "file_mode.h"
%}

%start    symbolic_mode
%%


symbolic_mode    : clause
                 | symbolic_mode ',' clause
                 ;


clause           : actionlist		{ mparse_clause(); }
                 | wholist actionlist	{ mparse_clause(); }
                 ;


wholist          : who
                 | wholist who
                 ;


who              : 'u'			{ mparse_who('u'); }
		 | 'g'			{ mparse_who('g'); }
		 | 'o'			{ mparse_who('o'); }
		 | 'a'			{ mparse_who('a'); }
                 ;


actionlist       : action		{ mparse_action(); }
                 | actionlist action	{ mparse_action(); }
                 ;


action           : op
                 | op permlist
                 | op permcopy
                 ;


permcopy         : 'u'			{ mparse_permcp('u'); }
		 | 'g'			{ mparse_permcp('g'); }
		 | 'o'			{ mparse_permcp('o'); }
                 ;


op               : '+'			{ mparse_op('+'); }
		 | '-'			{ mparse_op('-'); }
		 | '='			{ mparse_op('='); }
                 ;


permlist         : perm
                 | perm permlist
                 ;


perm             : 'r'			{ mparse_perm('r'); }
		 | 'w'			{ mparse_perm('w'); }
		 | 'x'			{ mparse_perm('x'); }
		 | 'X'			{ mparse_perm('X'); }
		 | 's'			{ mparse_perm('s'); }
		 | 't'			{ mparse_perm('t'); }
                 ;

