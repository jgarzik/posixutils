
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = grep

dist_man_MANS = grep.1

grep_SOURCES = grep.cc
grep_LDADD = $(LIBPU)

TESTS = grep-test

EXTRA_DIST = grep-test test1.out

