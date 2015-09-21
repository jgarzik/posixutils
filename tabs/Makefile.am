
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tabs

dist_man_MANS = tabs.1

tabs_SOURCES = tabs.cc
tabs_LDADD = $(LIBPU) $(CURSES_LIB)

TESTS = tabs-test

EXTRA_DIST = tabs-test

