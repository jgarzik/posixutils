
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = uuencode

dist_man_MANS = uuencode.1

uuencode_SOURCES = uuencode.cc
uuencode_LDADD = $(LIBPU)

TESTS = uuencode-test

EXTRA_DIST = uuencode-test

