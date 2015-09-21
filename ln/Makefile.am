
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = ln

dist_man_MANS = ln.1

ln_SOURCES = ln.cc
ln_LDADD = $(LIBPU)

TESTS = ln-test

EXTRA_DIST = ln-test

