
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = cmp

dist_man_MANS = cmp.1

cmp_SOURCES = cmp.cc
cmp_LDADD = $(LIBPU)

TESTS = cmp-test

EXTRA_DIST = cmp-test

