
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tput

dist_man_MANS = tput.1

tput_SOURCES = tput.cc
tput_LDADD = $(LIBPU) $(CURSES_LIB)

TESTS = tput-test

EXTRA_DIST = tput-test

