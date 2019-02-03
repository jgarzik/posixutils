
ACLOCAL_AMFLAGS = -I build-aux/m4

SUBDIRS = po

bin_PROGRAMS =
dist_man_MANS =
noinst_LIBRARIES =
noinst_PROGRAMS = 
noinst_HEADERS = include/libpu.h
noinst_HEADERS += signal-copy.h platform/signal/i686-linux-gnu.h platform/signal/x86_64-linux-gnu.h platform/signal/x86_64-darwin13.4.0.h
TESTS =
CLEANFILES =
EXTRA_DIST = doc/new-util.txt doc/opengroup-docs.txt doc/roadmap.txt
EXTRA_DIST += config.rpath mkinstalldirs TODO gen-strmap.pl testrunner.py

include asa/Makefile.include
include basename/Makefile.include
include cat/Makefile.include
include chown/Makefile.include
include cksum/Makefile.include
include cmp/Makefile.include
include comm/Makefile.include
include compress/Makefile.include
include date/Makefile.include
include df/Makefile.include
include diff/Makefile.include
include dirname/Makefile.include
include echo/Makefile.include
include env/Makefile.include
include expr/Makefile.include
include false/Makefile.include
include getconf/Makefile.include
include grep/Makefile.include
include head/Makefile.include
include id/Makefile.include
include ipcs/Makefile.include
include ipcrm/Makefile.include
include libpu/Makefile.include
include link/Makefile.include
include ln/Makefile.include
include logger/Makefile.include
include logname/Makefile.include
include mesg/Makefile.include
include mkdir/Makefile.include
include kill/Makefile.include
include mkfifo/Makefile.include
include mv/Makefile.include
include nice/Makefile.include
include nohup/Makefile.include
include pathchk/Makefile.include
include pax/Makefile.include
include pwd/Makefile.include
include renice/Makefile.include
include rm/Makefile.include
include rmdir/Makefile.include
include sleep/Makefile.include
include sort/Makefile.include
include split/Makefile.include
include strings/Makefile.include
include stty/Makefile.include
include tabs/Makefile.include
include tee/Makefile.include
include time/Makefile.include
include touch/Makefile.include
include tput/Makefile.include
include true/Makefile.include
include tsort/Makefile.include
include tty/Makefile.include
include uname/Makefile.include
include uniq/Makefile.include
include unlink/Makefile.include
include uuencode/Makefile.include
include wc/Makefile.include
include what/Makefile.include

install-exec-hook:
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	  $(DESTDIR)$(bindir)/uncompress$(EXEEXT)
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	  $(DESTDIR)$(bindir)/zcat$(EXEEXT)

uninstall-hook:
	rm $(DESTDIR)$(bindir)/zcat$(EXEEXT) \
	  $(DESTDIR)$(bindir)/uncompress$(EXEEXT)
