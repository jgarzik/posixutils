
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = what

dist_man_MANS = what.1

what_SOURCES = what.cc
what_LDADD = $(LIBPU)

TESTS = what-test

EXTRA_DIST = what-test

