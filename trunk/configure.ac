AC_INIT([scospub], [0.1], [ltolke@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_ARG_WITH(boinc, [  --with-boinc=/where/built where the boinc library is built], [], [
echo ERROR: Boinc path must be given. 1>&2
exit 1
])

AC_CONFIG_SRCDIR([application/wrapper.C])
AC_CONFIG_HEADER([config.h])

AC_SUBST(BOINC_DIR, [$with_boinc])
AC_SUBST(BOINC_API_DIR, [$with_boinc/api])
AC_SUBST(BOINC_LIB_DIR, [$with_boinc/lib])



# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.
AC_FUNC_FORK
AC_HEADER_STDC
AC_CHECK_FUNCS([memset])


AC_CONFIG_FILES(Makefile application/Makefile)
AC_OUTPUT
