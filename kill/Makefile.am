
LIBPU = ../libpu/libpu.a @ARGP_LIB@

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = kill

dist_man_MANS = kill.1

kill_SOURCES = kill.cc kill-siglist.h
kill_LDADD = $(LIBPU)

BUILT_SOURCES = kill-siglist.h
CLEANFILES = kill-siglist.h

TESTS = kill-test

EXTRA_DIST = kill-test gen-siglist.pl

kill-siglist.h: Makefile
	$(PERL) $(srcdir)/gen-siglist.pl $(top_srcdir)/signal-copy.h >$@
