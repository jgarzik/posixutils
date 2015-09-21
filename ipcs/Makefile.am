
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = ipcs

dist_man_MANS = ipcs.1

ipcs_SOURCES = ipcs.cc
ipcs_LDADD = $(LIBPU)

TESTS = ipcs-test

EXTRA_DIST = ipcs-test

