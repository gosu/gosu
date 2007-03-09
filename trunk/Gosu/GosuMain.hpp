//! \file GosuMain.hpp
//! Contains the interface to the GosuMain sub library.

#ifndef GOSU_MAIN_HPP
#define GOSU_MAIN_HPP

#include <string>

//! If you link against the GosuMain library, this is your new program entry
//! point.
int gosuMain();

namespace Gosu
{
    //! Returns the number of command line arguments given to the application.
	unsigned argumentCount();

    //! Returns the n-th command line argument given to the application.
	const std::wstring& argument(unsigned n);
}

#endif
