//! \file AutoLinkMain.hpp
//! Contains pragmas that make MSVC link against the right WinMain lib
//! automatically.

#ifdef _MSC_VER
#ifndef GOSU_AUTOLINKMAIN_HPP
#define GOSU_AUTOLINKMAIN_HPP

#ifdef _MT
    #ifdef NDEBUG
    #pragma comment(lib, "WinMainDyn.lib")
    #else
    #pragma comment(lib, "WinMainDebugDyn.lib")
    #endif
#else
    #ifdef NDEBUG
    #pragma comment(lib, "WinMain.lib")
    #else
    #pragma comment(lib, "WinMainDebug.lib")
    #endif
#endif

#endif
#endif
