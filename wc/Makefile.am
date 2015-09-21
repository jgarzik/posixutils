
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = wc

dist_man_MANS = wc.1

wc_SOURCES = wc.cc
wc_LDADD = $(LIBPU)

TESTS = wc-test

EXTRA_DIST = wc-test test.data

