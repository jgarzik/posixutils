
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = mesg

dist_man_MANS = mesg.1

mesg_SOURCES = mesg.cc
mesg_LDADD = $(LIBPU)

TESTS = mesg-test

EXTRA_DIST = mesg-test

