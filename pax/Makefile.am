
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = pax-options.data

BUILT_SOURCES = pax-options.h
CLEANFILES = pax-options.h

LIBPU = ../libpu/libpu.a

noinst_PROGRAMS = pax

pax_SOURCES = main.c pax.h pax.c ustar.c cpio.c zip.c
pax_LDADD = $(LIBPU) @ZLIB_LIB@

pax-options.h: Makefile
	$(PERL) $(top_srcdir)/gen-strmap.pl $(srcdir)/pax-options.data >$@
