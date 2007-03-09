//! \file AutoLink.hpp
//! Contains pragmas that make MSVC link against all the necessary libraries
//! automatically.

#ifdef _MSC_VER
#ifndef GOSU_AUTOLINK_HPP
#define GOSU_AUTOLINK_HPP

#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxerr8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "fmodvc.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")

#ifdef _MT
    #ifdef NDEBUG
    #pragma comment(lib, "GosuDyn.lib")
    #pragma comment(lib, "GosuAux_zlibDyn.lib")
    #else
    #pragma comment(lib, "GosuDebugDyn.lib")
    #pragma comment(lib, "GosuAux_zlibDebugDyn.lib")
    #endif
#else
    #ifdef NDEBUG
    #pragma comment(lib, "Gosu.lib")
    #pragma comment(lib, "GosuAux_zlib.lib")
    #else
    #pragma comment(lib, "GosuDebug.lib")
    #pragma comment(lib, "GosuAux_zlibDebug.lib")
    #endif
#endif

#endif
#endif
