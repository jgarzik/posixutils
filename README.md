
# POSIXUTILS

The posixutils package is a collection of the utilities specified in the
Single Unix Specification v3 (SuSv3).

## Mission

This project asks the question:

**What happens when the familiar shell utilities -- cp, rm, mv, etc. --
are written in modern C++, with full access to the C++11 compiler and
library?**

## Resources

posixutils home page (untouched for years):
	http://sourceforge.net/projects/nix-utils/

posixutils source code repository, git protocol:
	https://github.com/jgarzik/posixutils.git

## Specifications

The SuSv3 specification is available online at
http://www.opengroup.org/onlinepubs/009695399/toc.htm

and the full list of all utilities we plan to implement is at
http://www.opengroup.org/onlinepubs/009695399/idx/utilities.html

## Project goals/manifesto

* race-free userland (as near as possible given the POSIX file interface
  and the kernel, anyway)

* clean, maintainable codebase targetted at >=32-bit machines.
  avoid #ifdefs.  When a utility is not often used, such as
  compress(1), readability, and maintainability are considered more
  important than performance.

* correctness is always more important than performance.

* stay as close to the SuSv3 specification as possible. avoid frivolous
  feature addition, with two exceptions:
	a) common legacy features
	b) popular GNU utils features

* Initial OS target is Linux, and use of Linux-specific features is
  encouraged.  On the other hand, porting to another OS is also
  encouraged.

