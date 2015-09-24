
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = expr

dist_man_MANS = expr.1

expr_SOURCES = expr.cc expr-parse.y
expr_LDADD = $(LIBPU)

BUILT_SOURCES = expr-parse.h
AM_YFLAGS = -d

TESTS = expr-test

EXTRA_DIST = expr-test

