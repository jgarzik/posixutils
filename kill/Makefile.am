
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = kill

dist_man_MANS = kill.1

kill_SOURCES = kill.cc
kill_LDADD = $(LIBPU)

TESTS = kill-test

EXTRA_DIST = kill-test

