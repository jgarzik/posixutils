
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = env

dist_man_MANS = env.1

env_SOURCES = env.cc
env_LDADD = $(LIBPU)

TESTS = env-test

EXTRA_DIST = env-test

