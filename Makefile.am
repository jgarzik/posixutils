
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = m4 po include platform libpu

INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = config.rpath mkinstalldirs	TODO	\
	gen-siglist.pl gen-strmap.pl		\
	getconf-path.data getconf-system.data getconf-confstr.data

BUILT_SOURCES = kill-siglist.h getconf-path.h getconf-system.h getconf-confstr.h
CLEANFILES = kill-siglist.h getconf-path.h getconf-system.h getconf-confstr.h

LIBPU = libpu/libpu.a

bin_PROGRAMS = \
	asa basename cat cksum comm date df dirname echo false getconf	\
	grep head ipcrm	kill link logger logname ln mesg nice		\
	nohup pwd renice rm rmdir sleep tee touch true tty unlink	\
	uuencode wc what

# all programs listed here are works-in-progress (WIP)
# see file TODO for more information.
noinst_PROGRAMS = chown compress mkdir mkfifo mv pathchk split


asa_LDADD = $(LIBPU)

basename_LDADD = $(LIBPU)

cat_LDADD = $(LIBPU)

chown_LDADD = $(LIBPU)

cksum_SOURCES = cksum.c cksum_crctab.h
cksum_LDADD = $(LIBPU)

comm_LDADD = $(LIBPU)

compress_LDADD = $(LIBPU)

date_LDADD = $(LIBPU)

df_LDADD = $(LIBPU)

dirname_LDADD = $(LIBPU)

getconf_LDADD = $(LIBPU)

grep_LDADD = $(LIBPU)

head_LDADD = $(LIBPU)

ipcrm_LDADD = $(LIBPU)

kill_LDADD = $(LIBPU)

link_LDADD = $(LIBPU)

logger_LDADD = $(LIBPU)

logname_LDADD = $(LIBPU)

ln_LDADD = $(LIBPU)

mkdir_LDADD = $(LIBPU)

mkfifo_LDADD = $(LIBPU)

mesg_LDADD = $(LIBPU)

mv_LDADD = $(LIBPU)

nice_LDADD = $(LIBPU)

nohup_LDADD = $(LIBPU)

pathchk_LDADD = $(LIBPU)

pwd_LDADD = $(LIBPU)

renice_LDADD = $(LIBPU)

rm_LDADD = $(LIBPU)

rmdir_LDADD = $(LIBPU)

sleep_LDADD = $(LIBPU)

split_LDADD = $(LIBPU)

tee_LDADD = $(LIBPU)

touch_LDADD = $(LIBPU)

tty_LDADD = $(LIBPU)

unlink_LDADD = $(LIBPU)

uuencode_LDADD = $(LIBPU)

wc_LDADD = $(LIBPU)

what_LDADD = $(LIBPU)

kill-siglist.h: Makefile
	$(PERL) $(srcdir)/gen-siglist.pl signal-copy.h >$@
getconf-path.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-path.data >$@
getconf-system.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-system.data >$@
getconf-confstr.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-confstr.data >$@

