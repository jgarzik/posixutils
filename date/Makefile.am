
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = date

dist_man_MANS = date.1

date_SOURCES = date.cc
date_LDADD = $(LIBPU)

TESTS = date-test

EXTRA_DIST = date-test

