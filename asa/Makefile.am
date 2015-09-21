
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = asa

dist_man_MANS = asa.1

asa_SOURCES = asa.cc
asa_LDADD = $(LIBPU)

TESTS = asa-test

EXTRA_DIST = asa-test asa-test.in1 asa-test.out1

