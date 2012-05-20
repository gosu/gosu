//! \file AutoLink.hpp
//! Contains pragmas that make MSVC link against all the necessary libraries
//! automatically.

#ifdef _MSC_VER
#ifndef GOSU_AUTOLINK_HPP
#define GOSU_AUTOLINK_HPP

#if _MSC_VER >= 1600
#	ifdef NDEBUG
#		pragma comment(lib, "Gosuv100.lib")
#	else
#		pragma comment(lib, "GosuDebugv100.lib")
#	endif
#else
#	ifdef NDEBUG
#		pragma comment(lib, "Gosu.lib")
#	else
#		pragma comment(lib, "GosuDebug.lib")
#	endif
#endif

#endif
#endif
