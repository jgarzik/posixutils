
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = cat

dist_man_MANS = cat.1

cat_SOURCES = cat.cc
cat_LDADD = $(LIBPU)

TESTS = cat-test

EXTRA_DIST = cat-test

