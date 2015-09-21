
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = logname

dist_man_MANS = logname.1

logname_SOURCES = logname.cc
logname_LDADD = $(LIBPU)

TESTS = logname-test

EXTRA_DIST = logname-test

