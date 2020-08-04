# CHECK_GETOPT_OPTRESET
# --------------------------------------------------------------
# Set HAVE_GETOPT_OPTRESET if getopt() needs optreset to restart
AC_DEFUN([CHECK_GETOPT_OPTRESET],
[AC_CACHE_CHECK([whether getopt has optreset support],
		ac_cv_have_getopt_optreset, [
	AC_TRY_LINK(
		[
#include <getopt.h>
		],
		[ extern int optreset; optreset = 0; ],
		[ ac_cv_have_getopt_optreset="yes" ],
		[ ac_cv_have_getopt_optreset="no" ]
	)
])
if test "x$ac_cv_have_getopt_optreset" = "xyes" ; then
	AC_DEFINE(HAVE_GETOPT_OPTRESET, 1,
		[Define if your getopt(3) defines and uses optreset])
fi])
