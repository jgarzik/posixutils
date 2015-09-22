
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = chown

dist_man_MANS = chown.1

chown_SOURCES = chown.cc
chown_LDADD = $(LIBPU)

TESTS = chown-test

EXTRA_DIST = chown-test

