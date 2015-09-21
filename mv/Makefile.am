
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = mv

dist_man_MANS = mv.1

mv_SOURCES = mv.cc
mv_LDADD = $(LIBPU)

TESTS = mv-test

EXTRA_DIST = mv-test

