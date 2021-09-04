#pragma once

#if defined(WIN32) || defined(_WIN64)
# define GOSU_IS_WIN
#else
# define GOSU_IS_UNIX
# if defined(__linux) || defined(__FreeBSD__)
#  define GOSU_IS_X
# else
#  define GOSU_IS_MAC
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#   define GOSU_IS_IPHONE
#   define GOSU_IS_OPENGLES
#  endif
# endif
#endif

#ifndef GOSU_DEPRECATED
# define GOSU_DEPRECATED [[deprecated]]
#endif
