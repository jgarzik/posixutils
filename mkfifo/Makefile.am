
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = mkfifo

dist_man_MANS = mkfifo.1

mkfifo_SOURCES = mkfifo.cc
mkfifo_LDADD = $(LIBPU)

TESTS = mkfifo-test

EXTRA_DIST = mkfifo-test

