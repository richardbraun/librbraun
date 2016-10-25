AC_INIT([librbraun], [0.1], [rbraun@sceen.net], [librbraun])

AC_CONFIG_SRCDIR([macros.h])
AC_CONFIG_AUX_DIR([build-aux])

LT_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects 1.10])

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
                   [Size of the CPU cache line size defined as a base two logarithm])
AC_DEFINE_UNQUOTED([CONFIG_MEM_MALLOC], [0], [build malloc and friends])
AC_DEFINE_UNQUOTED([CONFIG_MEM_USE_PHYS], [0],
                   [make the mem module use the phys module as its backend])
AC_DEFINE_UNQUOTED([CONFIG_MEM_VERIFY], [0],
                   [enforce strong verification on all memory caches])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
