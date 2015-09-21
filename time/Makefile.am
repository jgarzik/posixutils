
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = time

dist_man_MANS = time.1

time_SOURCES = time.cc
time_LDADD = $(LIBPU)

TESTS = time-test

EXTRA_DIST = time-test

