dnl Process this file with autoconf to produce a configure script.
AC_INIT(posixutils, 3, [http://sourceforge.net/projects/nix-utils  ..])
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([cat.c])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([gnu])
AC_CONFIG_HEADERS([posixutils-config.h])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_RANLIB
AC_ISC_POSIX

AC_PATH_PROG(PERL, perl)

dnl Checks for libraries.
ALL_LINGUAS="fr"
AM_GNU_GETTEXT([external])

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

AC_DEFINE(_ISOC99_SOURCE, 1, [ISOC99 source])
AC_DEFINE(_POSIX_SOURCE, 1, [POSIX source])
AC_DEFINE(_POSIX_C_SOURCE, 199506L, [POSIX C source])
AC_DEFINE(_XOPEN_SOURCE, 600, [X/Open source])
AC_DEFINE(_XOPEN_SOURCE_EXTENDED, 1, [X/open extended source])
AC_DEFINE(_LARGEFILE64_SOURCE, 1, [largefile support])

AC_CONFIG_FILES([Makefile po/Makefile.in m4/Makefile 
		libpu/Makefile
		include/Makefile
		platform/Makefile
		platform/signal/Makefile])
AC_CONFIG_LINKS(signal-copy.h:platform/signal/$target_cpu-$target_os.h)
AC_OUTPUT


