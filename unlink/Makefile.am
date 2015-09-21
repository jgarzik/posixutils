
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = unlink

dist_man_MANS = unlink.1

unlink_SOURCES = unlink.cc
unlink_LDADD = $(LIBPU)

TESTS = unlink-test

EXTRA_DIST = unlink-test

