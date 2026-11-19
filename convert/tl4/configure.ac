AC_INIT( main.cc )
AM_INIT_AUTOMAKE( converter, 0.1 )
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

AC_CANONICAL_HOST
 
export CFLAGS="-Wall -g -O2"
export CXXFLAGS="-Wall -g -O2"

if test x$host = xi686-pc-cygwin; then
   AC_DEFINE( WINS, 1, Cygwin compile requires some messing )
fi

AC_OUTPUT( Makefile )
