
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = logger

dist_man_MANS = logger.1

logger_SOURCES = logger.cc
logger_LDADD = $(LIBPU)

TESTS = logger-test

EXTRA_DIST = logger-test

