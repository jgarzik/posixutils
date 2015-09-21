
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	doc m4 po include platform libpu \
	asa basename cat chown cksum cmp comm compress \
	date df dirname echo env false getconf grep head \
	id ipcrm ipcs kill link ln logger logname mesg mkdir \
	mkfifo mv nice nohup pathchk pax pwd renice rm rmdir \
	sleep split strings stty \
	tests

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = config.rpath mkinstalldirs	TODO gen-strmap.pl

LIBPU = libpu/libpu.a @ARGP_LIB@

bin_PROGRAMS = \
	tabs tee time	\
	touch tput true tsort tty uname unlink uuencode wc what

# all programs listed here are works-in-progress (WIP)
# see file TODO, and doc/status/PROGRAM.status, for more information.
noinst_PROGRAMS = uniq


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

