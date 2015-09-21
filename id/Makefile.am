
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = id

dist_man_MANS = id.1

id_SOURCES = id.cc
id_LDADD = $(LIBPU)

TESTS = id-test

EXTRA_DIST = id-test

