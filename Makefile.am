
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc m4 po include platform \
	libpu compress pax tests

INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = config.rpath mkinstalldirs	TODO	\
	gen-siglist.pl gen-strmap.pl		\
	getconf-path.data getconf-system.data getconf-confstr.data

BUILT_SOURCES = kill-siglist.h getconf-path.h getconf-system.h \
	getconf-confstr.h
CLEANFILES = kill-siglist.h getconf-path.h getconf-system.h \
	getconf-confstr.h

LIBPU = libpu/libpu.a

bin_PROGRAMS = \
	asa basename cat cksum cmp comm date df dirname echo env false	\
	getconf	grep head id ipcrm kill link logger logname ln mesg	\
	nice nohup pwd renice rm rmdir sleep strings tabs tee time	\
	touch tput true tty uname unlink uuencode wc what

# all programs listed here are works-in-progress (WIP)
# see file TODO, and doc/status/PROGRAM.status, for more information.
noinst_PROGRAMS = chown mkdir mkfifo mv pathchk split stty uniq


asa_LDADD = $(LIBPU)

basename_LDADD = $(LIBPU)

cat_LDADD = $(LIBPU)

chown_LDADD = $(LIBPU)

cksum_SOURCES = cksum.c cksum_crctab.h
cksum_LDADD = $(LIBPU)

cmp_LDADD = $(LIBPU)
comm_LDADD = $(LIBPU)

date_LDADD = $(LIBPU)

df_LDADD = $(LIBPU)

dirname_LDADD = $(LIBPU)

env_LDADD = $(LIBPU)

getconf_LDADD = $(LIBPU)

grep_LDADD = $(LIBPU)

head_LDADD = $(LIBPU)

id_LDADD = $(LIBPU)

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

strings_LDADD = $(LIBPU)

stty_LDADD = $(LIBPU)

tabs_LDADD = $(LIBPU) $(CURSES_LIB)

tee_LDADD = $(LIBPU)

time_LDADD = $(LIBPU)

tput_LDADD = $(LIBPU) $(CURSES_LIB)

touch_LDADD = $(LIBPU)

tty_LDADD = $(LIBPU)

uname_LDADD = $(LIBPU)

uniq_LDADD = $(LIBPU)

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

