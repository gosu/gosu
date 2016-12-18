cmake_minimum_required(VERSION 2.6)

project(Gosu)

set(GOSU_VERSION_MAJOR "0")
set(GOSU_VERSION_MINOR "10")
set(GOSU_VERSION_PATCH "8")
set(GOSU_VERSION "${GOSU_VERSION_MAJOR}.${GOSU_VERSION_MINOR}.${GOSU_VERSION_PATCH}")

option(CMAKE_VERBOSE_MAKEFILE "Display every compilation command" OFF)
if(WIN32)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()

option(CMAKE_BUILD_TYPE "Build type (Debug, Release, RelWithDebInfo or MinSizeRel)" RelWithDebInfo)

include(Compilation.cmake)
include(Installation.cmake)

include(Packaging.cmake)
