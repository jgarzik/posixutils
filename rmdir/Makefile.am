
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = rmdir

dist_man_MANS = rmdir.1

rmdir_SOURCES = rmdir.cc
rmdir_LDADD = $(LIBPU)

TESTS = rmdir-test

EXTRA_DIST = rmdir-test

