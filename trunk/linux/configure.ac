#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(Gosu, 0.6, jan.luecker@gmx.de)

GOSU_A_OBJS=\$\(BASE_OBJS\)
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(NET_OBJS)"

TO_BE_MADE="directories gosu.a examples gosu-config"

# Checks for programs.
AC_PROG_CXX
if test -z "$PKG_CONFIG"; then
  PKG_CONFIG=pkg-config
fi
AC_SUBST(PKG_CONFIG)

# Checks for libraries.
AC_CHECK_LIB([GL], [glMatrixMode], , AC_MSG_ERROR(Missing GL), -lpthread)
AC_CHECK_LIB([GLU], [gluNewQuadric], , AC_MSG_ERROR(Missing GLU), -lpthread)
AC_PATH_X
if test $no_x ; then
  AC_MSG_ERROR([Missing X])
fi

if test $x_includes; then
  X_INCS="-I"$x_includes
fi
AC_SUBST(X_INCS)
AC_SUBST(x_libraries)

LIBS="$LIBS -L$x_libraries"

#AC_CHECK_LIB([Xxf86vm], [XF86VidModeGetModeLine], ,
#  AC_MSG_ERROR(Missing Xxf86vm))

#AC_CHECK_LIB([png], [png_create_read_struct], , AC_MSG_ERROR(Missing libPNG))
# FIXME: Replace `main' with a function in `-lrt':
#AC_CHECK_LIB([rt], [main])

AC_MSG_CHECKING(for pangoft2)
pkg-config --exists pangoft2
if test $? -ne 0; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR(Missing pangoft2)
fi
AC_MSG_RESULT(yes)

PANGO_CXX_FLAGS=`pkg-config pangoft2 --cflags`
AC_SUBST(PANGO_CXX_FLAGS)
PANGO_LIBS=`pkg-config pangoft2 --libs`
AC_SUBST(PANGO_LIBS)

# Try some fmod.so variants
AC_MSG_CHECKING(for fmod)
AUDIO_LIBS="/usr/lib/libfmod.so"
FMOD_INC=""
if test ! -f $AUDIO_LIBS; then
  AUDIO_LIBS=`ls ../fmod*/api/libfmod*.so 2> /dev/null`
  FMOD_INC=-I`ls -d ../fmod*/api/inc 2> /dev/null`
fi
# If one is found, use AudioFmod
if test $AUDIO_LIBS; then
#  GOSU_AUDIO=AudioFmod.o
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(FMOD_OBJ)"
else
  AC_MSG_ERROR(Missing fmod)
fi

AC_MSG_RESULT(yes)
AC_SUBST(AUDIO_LIBS)
AC_SUBST(FMOD_INC)

AC_CHECK_PROGS(ruby, ruby1.9 ruby1.8 ruby)
RUBY_INC_DIR=`${ruby} -e 'require "rbconfig";print Config::CONFIG[["archdir"]]'`
RUBY_LIBS=-l`${ruby} -e 'require "rbconfig";print Config::CONFIG[["ruby_install_name"]]'`
# 2> /dev/null

if test $RUBY_INC_DIR; then
  AC_SUBST(RUBY_INC_DIR)
  AC_SUBST(RUBY_LIBS)
  TO_BE_MADE=${TO_BE_MADE}" ruby"
fi

AC_SUBST(GOSU_A_OBJS)
AC_SUBST(TO_BE_MADE)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#AC_CONFIG_FILES([Makefile_])
AC_OUTPUT(Makefile)

