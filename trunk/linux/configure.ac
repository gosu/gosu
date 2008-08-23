#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(Gosu, 0.7, jan.luecker@gmx.de)

GOSU_A_OBJS=\$\(BASE_OBJS\)
GOSU_A_OBJS=${GOSU_A_OBJS}" \$(NET_OBJS)"

TO_BE_MADE="directories gosu.a gosu.so examples gosu-config"

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

if test $x_libraries; then
  LIBS="$LIBS -L$x_libraries"
fi

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

# Try to find a suitable audio library.
AC_MSG_CHECKING(for fmod)
AUDIO_LIBS="/usr/lib/libfmod.so"
AUDIO_INC=""
if test ! -f $AUDIO_LIBS; then
  AUDIO_LIBS=`ls ../fmod*/api/libfmod*.so 2> /dev/null`
  AUDIO_INC=-I`ls -d ../fmod*/api/inc 2> /dev/null`
fi
# If one is found, use AudioFmod
if test $AUDIO_LIBS; then
  GOSU_A_OBJS=${GOSU_A_OBJS}" \$(FMOD_OBJ)"
  AC_MSG_RESULT(yes)
  AC_SUBST(AUDIO_LIBS)
  AC_SUBST(AUDIO_INC)
else
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(for SDL_mixer)
  if test -f "/usr/lib/libSDL_mixer.so" || test -f "/usr/local/lib/libSDL_mixer.so"; then
    GOSU_A_OBJS=${GOSU_A_OBJS}" \$(SDLMIXER_OBJ)"
    AUDIO_LIBS="`sdl-config --libs` -lSDL_mixer"
    AUDIO_INC=`sdl-config --cflags`
    AC_MSG_RESULT(yes)
    AC_SUBST(AUDIO_LIBS)
    AC_SUBST(AUDIO_INC)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(No suitable audio library found)
  fi
fi

AC_CHECK_HEADER(sys/stat.h, HAVE_SYS_STAT_H=-DHAVE_SYS_STAT_H)
AC_SUBST(HAVE_SYS_STAT_H)

# workaround ruby 1.9 redefining standard c++ eaccess
AC_CHECK_FUNC(eaccess, HAVE_EACCESS=-DHAVE_EACCESS)
AC_SUBST(HAVE_EACCESS)

AC_CHECK_PROGS(ruby, ruby1.8 ruby1.9 ruby)
RUBY_INC_DIR=`${ruby} -e 'require "rbconfig";print(Config::CONFIG[["rubyhdrdir"]] || Config::CONFIG[["archdir"]])'`
RUBY_ARCH_DIR=`${ruby} -e 'require "rbconfig";print File.join(Config::CONFIG[["rubyhdrdir"]], Config::CONFIG[["arch"]]) if Config::CONFIG[["rubyhdrdir"]]'`
RUBY_LIBS=`${ruby} -e 'require "rbconfig";print(Config::CONFIG[["LIBRUBYARG"]] || "-l" + Config::CONFIG[["ruby_install_name"]])'`
# 2> /dev/null

if test $RUBY_INC_DIR; then
  AC_SUBST(RUBY_INC_DIR)
  AC_SUBST(RUBY_ARCH_DIR)
  AC_SUBST(RUBY_LIBS)
  TO_BE_MADE=${TO_BE_MADE}" ruby"
fi

AC_CHECK_PROG(HAVE_SWIG, swig, yes, no)
 
if test $HAVE_SWIG != "yes"; then
  AC_MSG_ERROR(Cannot find swig)
fi

AC_CHECK_LIB(dl, dlopen, LIBS="$LIBS -ldl", AC_CHECK_LIB(c, dlopen, ,AC_MSG_ERROR(Missing dlopen functionality), ), )

AC_SUBST(DLOPEN_LIB)
AC_SUBST(GOSU_A_OBJS)
AC_SUBST(TO_BE_MADE)

AC_CHECK_LIB(rt, sched_setscheduler, LIBS="$LIBS -lrt", AC_CHECK_LIB(c, sched_setscheduler, , AC_MSG_ERROR(Missing librt functionality), ), )
AC_SUBST(HAVE_LIBRT)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#AC_CONFIG_FILES([Makefile_])
AC_OUTPUT(Makefile)

