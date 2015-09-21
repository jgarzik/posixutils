
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc m4 po include platform \
	libpu compress pax tests

AM_CPPFLAGS = -I$(top_srcdir)/include

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
	touch tput true tsort tty uname unlink uuencode wc what

# all programs listed here are works-in-progress (WIP)
# see file TODO, and doc/status/PROGRAM.status, for more information.
noinst_PROGRAMS = chown ipcs mkdir mkfifo mv pathchk split stty uniq


asa_SOURCES = asa.cc
asa_LDADD = $(LIBPU)

basename_SOURCES = basename.cc
basename_LDADD = $(LIBPU)

cat_SOURCES = cat.cc
cat_LDADD = $(LIBPU)

chown_SOURCES = chown.cc
chown_LDADD = $(LIBPU)

cksum_SOURCES = cksum.cc cksum_crctab.h
cksum_LDADD = $(LIBPU)

cmp_SOURCES = cmp.cc
cmp_LDADD = $(LIBPU)
comm_SOURCES = comm.cc
comm_LDADD = $(LIBPU)

date_SOURCES = date.cc
date_LDADD = $(LIBPU)

df_SOURCES = df.cc
df_LDADD = $(LIBPU)

dirname_SOURCES = dirname.cc
dirname_LDADD = $(LIBPU)

echo_SOURCES = echo.cc

env_SOURCES = env.cc
env_LDADD = $(LIBPU)

getconf_SOURCES = getconf.cc
getconf_LDADD = $(LIBPU)

grep_SOURCES = grep.cc
grep_LDADD = $(LIBPU)

head_SOURCES = head.cc
head_LDADD = $(LIBPU)

id_SOURCES = id.cc
id_LDADD = $(LIBPU)

ipcs_SOURCES = ipcs.cc
ipcs_LDADD = $(LIBPU)

ipcrm_SOURCES = ipcrm.cc
ipcrm_LDADD = $(LIBPU)

kill_SOURCES = kill.cc
kill_LDADD = $(LIBPU)

link_SOURCES = link.cc
link_LDADD = $(LIBPU)

logger_SOURCES = logger.cc
logger_LDADD = $(LIBPU)

logname_SOURCES = logname.cc
logname_LDADD = $(LIBPU)

ln_SOURCES = ln.cc
ln_LDADD = $(LIBPU)

mkdir_SOURCES = mkdir.cc
mkdir_LDADD = $(LIBPU)

mkfifo_SOURCES = mkfifo.cc
mkfifo_LDADD = $(LIBPU)

mesg_SOURCES = mesg.cc
mesg_LDADD = $(LIBPU)

mv_SOURCES = mv.cc
mv_LDADD = $(LIBPU)

nice_SOURCES = nice.cc
nice_LDADD = $(LIBPU)

nohup_SOURCES = nohup.cc
nohup_LDADD = $(LIBPU)

pathchk_SOURCES = pathchk.cc
pathchk_LDADD = $(LIBPU)

pwd_SOURCES = pwd.cc
pwd_LDADD = $(LIBPU)

renice_SOURCES = renice.cc
renice_LDADD = $(LIBPU)

rm_SOURCES = rm.cc
rm_LDADD = $(LIBPU)

rmdir_SOURCES = rmdir.cc
rmdir_LDADD = $(LIBPU)

sleep_SOURCES = sleep.cc
sleep_LDADD = $(LIBPU)

split_SOURCES = split.cc
split_LDADD = $(LIBPU)

strings_SOURCES = strings.cc
strings_LDADD = $(LIBPU)

stty_SOURCES = stty.cc
stty_LDADD = $(LIBPU)

tabs_SOURCES = tabs.cc
tabs_LDADD = $(LIBPU) $(CURSES_LIB)

tee_SOURCES = tee.cc
tee_LDADD = $(LIBPU)

time_SOURCES = time.cc
time_LDADD = $(LIBPU)

tput_SOURCES = tput.cc
tput_LDADD = $(LIBPU) $(CURSES_LIB)

touch_SOURCES = touch.cc
touch_LDADD = $(LIBPU)

tsort_SOURCES = tsort.cc
tsort_LDADD = $(LIBPU)

tty_SOURCES = tty.cc
tty_LDADD = $(LIBPU)

uname_SOURCES = uname.cc
uname_LDADD = $(LIBPU)

uniq_SOURCES = uniq.cc
uniq_LDADD = $(LIBPU)

unlink_SOURCES = unlink.cc
unlink_LDADD = $(LIBPU)

uuencode_SOURCES = uuencode.cc
uuencode_LDADD = $(LIBPU)

wc_SOURCES = wc.cc
wc_LDADD = $(LIBPU)

what_SOURCES = what.cc
what_LDADD = $(LIBPU)

kill-siglist.h: Makefile
	$(PERL) $(srcdir)/gen-siglist.pl signal-copy.h >$@
getconf-path.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-path.data >$@
getconf-system.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-system.data >$@
getconf-confstr.h: Makefile
	$(PERL) $(srcdir)/gen-strmap.pl $(srcdir)/getconf-confstr.data >$@

