
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = touch

dist_man_MANS = touch.1

touch_SOURCES = touch.cc
touch_LDADD = $(LIBPU)

TESTS = touch-test

EXTRA_DIST = touch-test

