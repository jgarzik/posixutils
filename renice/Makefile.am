
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = renice

dist_man_MANS = renice.1

renice_SOURCES = renice.cc
renice_LDADD = $(LIBPU)

TESTS = renice-test

EXTRA_DIST = renice-test

