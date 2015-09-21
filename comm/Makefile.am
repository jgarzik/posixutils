
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = comm

dist_man_MANS = comm.1

comm_SOURCES = comm.cc
comm_LDADD = $(LIBPU)

TESTS = comm-test

EXTRA_DIST = comm-test

