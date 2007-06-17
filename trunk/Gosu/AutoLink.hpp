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


#ifdef NDEBUG
#pragma comment(lib, "Gosu.lib")
#else
#pragma comment(lib, "GosuDebug.lib")
#endif

#endif
#endif
