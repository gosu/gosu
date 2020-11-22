//! \file Platform.hpp
//! Macros and utility functions to facilitate programming on all of Gosu's supported platforms.

#pragma once

#ifdef __BIG_ENDIAN__
# define GOSU_IS_BIG_ENDIAN
# define IDENTITY_FUN big_to_native
# define IDENTITY_FUN2 native_to_big
# define CONV_FUN little_to_native
# define CONV_FUN2 native_to_little
#else
# define GOSU_IS_LITTLE_ENDIAN
# define IDENTITY_FUN little_to_native
# define IDENTITY_FUN2 native_to_little
# define CONV_FUN big_to_native
# define CONV_FUN2 native_to_big
#endif

#include <algorithm>

namespace Gosu
{
    template<typename T> T IDENTITY_FUN(T t) { return t; }
    template<typename T> T IDENTITY_FUN2(T t) { return t; }
    
    template<typename T>
    T CONV_FUN(T t)
    {
        char* begin = reinterpret_cast<char*>(&t);
        std::reverse(begin, begin + sizeof t);
        return t;
    }

    template<typename T> T CONV_FUN2(T t) { return CONV_FUN(t); }
}

#undef IDENTITY_FUN
#undef IDENTITY_FUN2
#undef CONV_FUN
#undef CONV_FUN2

#if defined(_MSC_VER)
# define GOSU_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
# define GOSU_NORETURN __attribute__ ((noreturn))
#endif

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
# if defined(GOSU_IS_WIN)
#  define GOSU_DEPRECATED __declspec(deprecated)
# else
#  define GOSU_DEPRECATED __attribute__((__deprecated__))
# endif
#endif
