
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = sort

dist_man_MANS = sort.1

sort_SOURCES = sort.cc
sort_LDADD = $(LIBPU)

TESTS = sort-test

EXTRA_DIST = sort-test test.data test.t1 test.t2

