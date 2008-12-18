
#ifndef __FILE_MODE_H__
#define __FILE_MODE_H__

/* parser callbacks */
extern void mparse_who(int);
extern void mparse_permcp(int);
extern void mparse_perm(int);
extern void mparse_op(int);
extern void mparse_clause(void);
extern void mparse_action(void);

/* standard yacc parser interface */
extern int mode_lex(void);
extern int mode_error(const char *s);
extern int mode_parse(void);

#endif /* __FILE_MODE_H__ */
