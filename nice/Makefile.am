
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = nice

dist_man_MANS = nice.1

nice_SOURCES = nice.cc
nice_LDADD = $(LIBPU)

TESTS = nice-test

EXTRA_DIST = nice-test

