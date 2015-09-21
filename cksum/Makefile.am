
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = cksum

dist_man_MANS = cksum.1

cksum_SOURCES = cksum.cc cksum_crctab.h
cksum_LDADD = $(LIBPU)

TESTS = cksum-test

EXTRA_DIST = cksum-test cksum-test.data test.data

