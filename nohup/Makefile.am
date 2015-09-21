
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = nohup

dist_man_MANS = nohup.1

nohup_SOURCES = nohup.cc
nohup_LDADD = $(LIBPU)

TESTS = nohup-test

EXTRA_DIST = nohup-test

