# Assumptions:
# - We cannot go higher than CMake 3.10 while supporting Ubuntu 18.04.
# - Prefer static libraries. Users should link Gosu into their games.
# - This is not (yet?) going to be used for building Ruby/Gosu.
# - This is not (yet?) going to replace the Visual Studio project for Windows.
cmake_minimum_required(VERSION 3.10)

project(Gosu)

set(GOSU_VERSION_MAJOR "0")
set(GOSU_VERSION_MINOR "15")
set(GOSU_VERSION_PATCH "1")
set(GOSU_VERSION "${GOSU_VERSION_MAJOR}.${GOSU_VERSION_MINOR}.${GOSU_VERSION_PATCH}")

option(CMAKE_VERBOSE_MAKEFILE "Display every compilation command" OFF)
if(WIN32)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()

option(CMAKE_BUILD_TYPE "Build type (Debug, Release, RelWithDebInfo or MinSizeRel)" RelWithDebInfo)

include(cmake/Compilation.cmake)
include(cmake/Installation.cmake)
