
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = getconf

dist_man_MANS = getconf.1

getconf_SOURCES = getconf.cc getconf-path.h getconf-system.h getconf-confstr.h
getconf_LDADD = $(LIBPU)

TESTS = getconf-test

EXTRA_DIST = getconf-test \
	getconf-path.data getconf-system.data getconf-confstr.data

BUILT_SOURCES = getconf-path.h getconf-system.h getconf-confstr.h
CLEANFILES = getconf-path.h getconf-system.h getconf-confstr.h

getconf-path.h: Makefile
	$(PERL) $(top_srcdir)/gen-strmap.pl $(srcdir)/getconf-path.data >$@
getconf-system.h: Makefile
	$(PERL) $(top_srcdir)/gen-strmap.pl $(srcdir)/getconf-system.data >$@
getconf-confstr.h: Makefile
	$(PERL) $(top_srcdir)/gen-strmap.pl $(srcdir)/getconf-confstr.data >$@

