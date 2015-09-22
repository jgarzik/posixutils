
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = split

dist_man_MANS = split.1

split_SOURCES = split.cc
split_LDADD = $(LIBPU)

TESTS = split-test

EXTRA_DIST = split-test

