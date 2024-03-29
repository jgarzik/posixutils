dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])
AC_INIT([posixutils],[2019.02.10],[http://sourceforge.net/projects/nix-utils  ..])
AC_CONFIG_SRCDIR([true/true.c])
AC_CONFIG_HEADERS([posixutils-config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux/m4])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([foreign subdir-objects])

AM_MAINTAINER_MODE([enable])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_RANLIB
AC_SEARCH_LIBS([strerror],[cposix])

dnl Require C++11 compiler (no GNU extensions)
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory], [nodefault])

AC_PATH_PROG(PERL, perl)

AM_PATH_PYTHON
AM_PROG_CC_C_O

dnl Checks for libraries.
ALL_LINGUAS="fr"
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.13])

dnl Checks for header files.
AC_C_BIGENDIAN

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_PID_T

AC_LANG(C++)

dnl Checks for library functions.
ZLIB_LIB=""
CURSES_LIB=""

AC_CHECK_FUNCS(__fsetlocking fgets_unlocked posix_fadvise)

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

AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_CONFIG_LINKS(signal-copy.h:platform/signal/$target_cpu-$target_os.h)
AC_OUTPUT

