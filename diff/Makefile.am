
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

# noinst=WIP
noinst_PROGRAMS = diff

dist_man_MANS = diff.1

diff_SOURCES = diff.cc
diff_LDADD = $(LIBPU)

TESTS = diff-test

EXTRA_DIST = diff-test

