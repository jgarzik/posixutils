
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = pathchk

dist_man_MANS = pathchk.1

pathchk_SOURCES = pathchk.cc
pathchk_LDADD = $(LIBPU)

TESTS = pathchk-test

EXTRA_DIST = pathchk-test

