AC_INIT([librbraun], [0.1], [rbraun@sceen.net], [librbraun])

AC_CONFIG_SRCDIR([macros.h])
AC_CONFIG_AUX_DIR([build-aux])

LT_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects 1.11])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])],
         [AC_SUBST([AM_DEFAULT_VERBOSITY], [1])])

AC_PREFIX_DEFAULT([])
AC_CANONICAL_HOST
AC_PROG_CPP
AC_PROG_CC
AM_PROG_AS
AM_PROG_CC_C_O

AC_HEADER_ASSERT()

AC_DEFINE_UNQUOTED([CONFIG_NR_CPUS], [8],
                   [maximum number of supported processors])
AC_DEFINE_UNQUOTED([CONFIG_CPU_L1_SHIFT], [6],
                   [processor cache line size as a base two exponent])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
