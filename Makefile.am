
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	doc m4 po include platform libpu \
	asa basename cat chown cksum cmp comm compress \
	date df dirname echo env false getconf grep head \
	id ipcrm ipcs kill link ln logger logname mesg mkdir \
	mkfifo mv nice nohup pax tests

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = config.rpath mkinstalldirs	TODO gen-strmap.pl

LIBPU = libpu/libpu.a @ARGP_LIB@

bin_PROGRAMS = \
	pwd renice rm rmdir sleep strings tabs tee time	\
	touch tput true tsort tty uname unlink uuencode wc what

# all programs listed here are works-in-progress (WIP)
# see file TODO, and doc/status/PROGRAM.status, for more information.
noinst_PROGRAMS = pathchk split stty uniq


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

