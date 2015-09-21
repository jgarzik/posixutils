
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = link

dist_man_MANS = link.1

link_SOURCES = link.cc
link_LDADD = $(LIBPU)

TESTS = link-test

EXTRA_DIST = link-test

