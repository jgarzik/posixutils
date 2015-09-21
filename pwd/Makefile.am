
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = pwd

dist_man_MANS = pwd.1

pwd_SOURCES = pwd.cc
pwd_LDADD = $(LIBPU)

TESTS = pwd-test

EXTRA_DIST = pwd-test

