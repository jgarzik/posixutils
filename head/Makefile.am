
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = head

dist_man_MANS = head.1

head_SOURCES = head.cc
head_LDADD = $(LIBPU)

TESTS = head-test

EXTRA_DIST = head-test

