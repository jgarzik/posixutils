
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = sleep

dist_man_MANS = sleep.1

sleep_SOURCES = sleep.cc
sleep_LDADD = $(LIBPU)

TESTS = sleep-test

EXTRA_DIST = sleep-test

