
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = uniq

dist_man_MANS = uniq.1

uniq_SOURCES = uniq.cc
uniq_LDADD = $(LIBPU)

TESTS = uniq-test

EXTRA_DIST = uniq-test uniq.data1 uniq.data2

