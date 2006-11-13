
bin_PROGRAMS = compress

compress_SOURCES = zopen.c zopen.h compress.c

EXTRA_DIST = NOTES README revision.log

install-exec-hook:
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	   $(DESTDIR)$(bindir)/uncompress$(EXEEXT)
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	   $(DESTDIR)$(bindir)/zcat$(EXEEXT)

