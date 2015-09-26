
AM_CPPFLAGS = -I$(top_srcdir)/include -DLOCALEDIR=\"@datadir@/locale\"

noinst_LIBRARIES = libpu.a

noinst_PROGRAMS = libpu-tests

libpu_a_SOURCES = \
	alloc.cc cmdline.cc lib.cc path.cc regex.cc \
	slist.cc strsplit.cc term.cc walk.cc

libpu_tests_SOURCES = libpu-tests.cc
libpu_tests_LDADD = libpu.a

TESTS = libpu-tests
