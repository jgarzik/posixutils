
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = df

dist_man_MANS = df.1

df_SOURCES = df.cc
df_LDADD = $(LIBPU)

TESTS = df-test

EXTRA_DIST = df-test

