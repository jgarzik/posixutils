
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = rm

dist_man_MANS = rm.1

rm_SOURCES = rm.cc
rm_LDADD = $(LIBPU)

TESTS = rm-test

EXTRA_DIST = rm-test

