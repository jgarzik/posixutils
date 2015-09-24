
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = basename

dist_man_MANS = basename.1

basename_SOURCES = basename.cc
basename_LDADD = $(LIBPU)

TESTS = basename-test

EXTRA_DIST = basename-test basename-tests.json

