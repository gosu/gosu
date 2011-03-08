#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(Gosu, 0.7, julian@raschke.de)

GOSU_A_OBJS=\$\(BASE_OBJS\)
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(NET_OBJS)"

TO_BE_MADE="directories ../lib/libgosu.a ../lib/libgosu.so examples ../bin/gosu-config"

# pkg-config...whatever that is.
AC_PROG_CXX
if test -z "$PKG_CONFIG"; then
  PKG_CONFIG=pkg-config
fi
AC_SUBST(PKG_CONFIG)

# OpenGL.
AC_CHECK_LIB([GL], [glMatrixMode], , AC_MSG_ERROR(Missing GL), -lpthread)

# X11 libraries...
AC_PATH_X
if test $no_x ; then
  AC_MSG_ERROR([Missing X])
fi
if test $x_includes; then
  X_INCS="-I"$x_includes
fi
AC_SUBST(X_INCS)
AC_SUBST(x_libraries)
if test $x_libraries; then
  LIBS="$LIBS -L$x_libraries"
fi

# Pango/Freetype
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

#AC_MSG_CHECKING(for SDL_mixer)
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(SDLMIXER_OBJ)"
AUDIO_LIBS="`sdl-config --libs` -lSDL_mixer"
AUDIO_INC=`sdl-config --cflags`
#AC_MSG_RESULT(yes)
AC_SUBST(AUDIO_LIBS)
AC_SUBST(AUDIO_INC)

# stat.h
AC_CHECK_HEADER(sys/stat.h, HAVE_SYS_STAT_H=-DHAVE_SYS_STAT_H)
AC_SUBST(HAVE_SYS_STAT_H)

# libdl
AC_CHECK_LIB(dl, dlopen, LIBS="$LIBS -ldl", AC_CHECK_LIB(c, dlopen, ,AC_MSG_ERROR(Missing dlopen functionality), ), )
AC_SUBST(DLOPEN_LIB)

AC_SUBST(GOSU_A_OBJS)
AC_SUBST(TO_BE_MADE)

# librt
AC_CHECK_LIB(rt, sched_setscheduler, LIBS="$LIBS -lrt", AC_CHECK_LIB(c, sched_setscheduler, , AC_MSG_ERROR(Missing librt functionality), ), )
AC_SUBST(HAVE_LIBRT)

INSTALL_ACTION="cpp_install"
UNINSTALL_ACTION="cpp_uninstall"
AC_SUBST(INSTALL_ACTION)
AC_SUBST(UNINSTALL_ACTION)

AC_OUTPUT(Makefile)
