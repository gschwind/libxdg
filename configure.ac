AC_PREREQ([2.69])
AC_INIT([libxdg], [0.0.1])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([src/main.cxx])
AM_MAINTAINER_MODE([disable])

AM_PROG_AR
LT_INIT

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  libxdg.pc
  ])
AC_OUTPUT
