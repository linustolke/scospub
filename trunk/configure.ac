# Because of BOINC the version number must be n.m where m<100
AC_INIT([scospub], [0.12], [ltolke@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_ARG_WITH(boinc,
            AC_HELP_STRING([--with-boinc=/where/built],
	                   [where the boinc library is built]),
            [
              if test "$withval" = yes
              then
                AC_MSG_ERROR(Boinc source must be explicitly given)
              elif test ! -r $withval/api/libboinc_api.a
              then
                AC_MSG_ERROR($withval does not point to built boinc project) 
              fi
            ],
            AC_MSG_ERROR(Boinc path must be given.)
           )

AC_SUBST(BOINC_DIR, [$with_boinc])

AC_ARG_WITH(project,
            AC_HELP_STRING([--with-project=/where/projects/project],
                           [where the project to install in is located]),
            [
              if test "$withval" = yes
              then
                AC_MSG_ERROR(Project must be explicitly given)
              elif test ! -r $withval/html/inc/db.inc
              then
                AC_MSG_ERROR($withval does not point to an installed boinc project) 
              fi
            ],
            AC_MSG_ERROR(Project path must be given.)
           )

AC_SUBST(PROJECT_DIR, [$with_project])


AC_CONFIG_SRCDIR([application/wrapper.C])
AC_CONFIG_HEADER([config.h])

AC_CANONICAL_HOST()

# Checks for programs.
AC_PROG_CXX

[
if [ $GXX = yes ]; then
  ln -sf `g++ -print-file-name=libstdc++.a` application
fi
]

AC_CHECK_MYSQL
if test "${no_mysql}" = yes; then
  AC_MSG_ERROR([ mysql not found.])
fi

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


AC_CONFIG_FILES(Makefile
	        application/Makefile
                tools/Makefile
		wu/Makefile
                html/Makefile
                html/scospub.httpd.conf
                html/languages/Makefile
                html/languages/project_specific_translations/Makefile
		html/scosconf/Makefile
		html/scospres/Makefile)
AC_OUTPUT
