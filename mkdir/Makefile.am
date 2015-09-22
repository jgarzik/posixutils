
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = mkdir

dist_man_MANS = mkdir.1

mkdir_SOURCES = mkdir.cc
mkdir_LDADD = $(LIBPU)

TESTS = mkdir-test

EXTRA_DIST = mkdir-test

