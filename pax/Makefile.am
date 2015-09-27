
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = pax-options.data pax-test

BUILT_SOURCES = pax-options.h
CLEANFILES = pax-options.h

LIBPU = ../libpu/libpu.a

noinst_PROGRAMS = pax

pax_SOURCES = main.cc pax.h pax.cc ustar.cc cpio.cc zip.cc
pax_LDADD = $(LIBPU) @ZLIB_LIB@ @ARGP_LIB@

pax-options.h: Makefile
	$(PERL) $(top_srcdir)/gen-strmap.pl $(srcdir)/pax-options.data >$@

TESTS = pax-test

