
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tee

dist_man_MANS = tee.1

tee_SOURCES = tee.cc
tee_LDADD = $(LIBPU)

TESTS = tee-test

EXTRA_DIST = tee-test

