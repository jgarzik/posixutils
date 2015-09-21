
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = dirname

dist_man_MANS = dirname.1

dirname_SOURCES = dirname.cc
dirname_LDADD = $(LIBPU)

TESTS = dirname-test

EXTRA_DIST = dirname-test

