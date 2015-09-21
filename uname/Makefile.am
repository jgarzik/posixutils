
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = uname

dist_man_MANS = uname.1

uname_SOURCES = uname.cc
uname_LDADD = $(LIBPU)

TESTS = uname-test

EXTRA_DIST = uname-test

