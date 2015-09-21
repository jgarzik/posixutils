dnl Process this file with autoconf to produce a configure script.
AC_INIT(posixutils, 5git, [http://sourceforge.net/projects/nix-utils  ..])
AC_PREREQ(2.60)
AC_CONFIG_SRCDIR([true/true.c])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([gnu])
AC_CONFIG_HEADERS([posixutils-config.h])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_ISC_POSIX

AC_PATH_PROG(PERL, perl)

dnl Checks for libraries.
ALL_LINGUAS="fr"
AM_GNU_GETTEXT([external])

dnl Checks for header files.
AC_HEADER_STDC
AC_C_BIGENDIAN

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
ZLIB_LIB=""
CURSES_LIB=""

AC_CHECK_FUNCS(__fsetlocking fgets_unlocked)

AC_CHECK_LIB(z,inflate,ZLIB_LIB="-lz",exit 1)
AC_CHECK_LIB(ncurses,setupterm,CURSES_LIB="-lncurses",exit 1)
AC_CHECK_LIB(argp, argp_parse, ARGP_LIB=-largp)

AC_DEFINE(_ISOC99_SOURCE, 1, [ISOC99 source])
AC_DEFINE(_POSIX_SOURCE, 1, [POSIX source])
AC_DEFINE(_POSIX_C_SOURCE, 200809L, [POSIX C source])
AC_DEFINE(_XOPEN_SOURCE, 700, [X/Open source])
AC_DEFINE(_XOPEN_SOURCE_EXTENDED, 1, [X/open extended source])
AC_DEFINE(_LARGEFILE64_SOURCE, 1, [largefile support])

AC_SUBST(ZLIB_LIB)
AC_SUBST(CURSES_LIB)
AC_SUBST(ARGP_LIB)

AC_CONFIG_FILES([Makefile
		po/Makefile.in
		doc/Makefile
		doc/status/Makefile
		m4/Makefile 
		libpu/Makefile
		include/Makefile
		asa/Makefile
		basename/Makefile
		cat/Makefile
		chown/Makefile
		cksum/Makefile
		cmp/Makefile
		comm/Makefile
		compress/Makefile
		date/Makefile
		df/Makefile
		dirname/Makefile
		echo/Makefile
		env/Makefile
		false/Makefile
		getconf/Makefile
		grep/Makefile
		head/Makefile
		id/Makefile
		ipcrm/Makefile
		ipcs/Makefile
		kill/Makefile
		link/Makefile
		ln/Makefile
		logger/Makefile
		logname/Makefile
		mesg/Makefile
		mkdir/Makefile
		mkfifo/Makefile
		mv/Makefile
		nice/Makefile
		nohup/Makefile
		pax/Makefile
		pathchk/Makefile
		pwd/Makefile
		renice/Makefile
		rm/Makefile
		rmdir/Makefile
		sleep/Makefile
		split/Makefile
		strings/Makefile
		stty/Makefile
		tabs/Makefile
		tee/Makefile
		time/Makefile
		touch/Makefile
		tput/Makefile
		true/Makefile
		tsort/Makefile
		tty/Makefile
		uname/Makefile
		uniq/Makefile
		unlink/Makefile
		uuencode/Makefile
		wc/Makefile
		what/Makefile
		platform/Makefile
		platform/signal/Makefile])
AC_CONFIG_LINKS(signal-copy.h:platform/signal/$target_cpu-$target_os.h)
AC_OUTPUT


