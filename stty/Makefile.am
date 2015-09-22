
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = stty

dist_man_MANS = stty.1

stty_SOURCES = stty.cc
stty_LDADD = $(LIBPU)

TESTS = stty-test

EXTRA_DIST = stty-test

