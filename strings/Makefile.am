
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = strings

dist_man_MANS = strings.1

strings_SOURCES = strings.cc
strings_LDADD = $(LIBPU)

TESTS = strings-test

EXTRA_DIST = strings-test

