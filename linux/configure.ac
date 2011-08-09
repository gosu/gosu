#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(Gosu, 0.7, julian@raschke.de)

GOSU_A_OBJS=\$\(BASE_OBJS\)
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(NET_OBJS)"

TO_BE_MADE="directories ../lib/libgosu.a ../lib/libgosu.so examples ../bin/gosu-config"

# pkg-config
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
AC_MSG_CHECKING(for libpangoft2)
pkg-config --exists pangoft2
if test $? -ne 0; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR(Missing libpangoft2)
fi
AC_MSG_RESULT(yes)

PANGO_CXX_FLAGS=`pkg-config pangoft2 --cflags`
AC_SUBST(PANGO_CXX_FLAGS)
PANGO_LIBS=`pkg-config pangoft2 --libs`
AC_SUBST(PANGO_LIBS)

# OpenAL
AC_MSG_CHECKING(for libopenal)
pkg-config --exists openal
if test $? -ne 0; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR(Missing libopenal)
fi
AC_MSG_RESULT(yes)

# libvorbisfile
AC_MSG_CHECKING(for libvorbisfile)
pkg-config --exists vorbisfile
if test $? -ne 0; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR(Missing vorbisfile some distros carry it in the vorbis package)
fi
AC_MSG_RESULT(yes)

# libsndfile
AC_MSG_CHECKING(for libsndfile)
pkg-config --exists sndfile
if test $? -ne 0; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR(Missing libsndfile)
fi
AC_MSG_RESULT(yes)

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

# libXdamage
AC_CHECK_HEADER(X11/extensions/Xdamage.h, [], AC_MSG_ERROR([Missing libxdamage]))

# libfreeimage
AC_CHECK_HEADER(FreeImage.h, [], AC_MSG_ERROR([Missing libfreeimage]))

INSTALL_ACTION="cpp_install"
UNINSTALL_ACTION="cpp_uninstall"
AC_SUBST(INSTALL_ACTION)
AC_SUBST(UNINSTALL_ACTION)

AC_OUTPUT(Makefile)
