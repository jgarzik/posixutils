
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tsort

dist_man_MANS = tsort.1

tsort_SOURCES = tsort.cc
tsort_LDADD = $(LIBPU)

TESTS = tsort-test

EXTRA_DIST = tsort-test tsort.data1 tsort.data2

