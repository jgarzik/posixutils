
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = echo

dist_man_MANS = echo.1

echo_SOURCES = echo.cc
echo_LDADD = $(LIBPU)

TESTS = echo-test

EXTRA_DIST = echo-test

