
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = ipcrm

dist_man_MANS = ipcrm.1

ipcrm_SOURCES = ipcrm.cc
ipcrm_LDADD = $(LIBPU)

TESTS = ipcrm-test

EXTRA_DIST = ipcrm-test

