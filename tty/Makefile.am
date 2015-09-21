
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tty

dist_man_MANS = tty.1

tty_SOURCES = tty.cc
tty_LDADD = $(LIBPU)

TESTS = tty-test

EXTRA_DIST = tty-test

