# Structure inspired by https://github.com/pabloariasal/modern-cmake-sample
# Assumptions:
# - We cannot go higher than CMake 3.10 while we support Ubuntu 18.04.
# - Prefer static libraries. Users should link Gosu into their games.
# - This is not (yet?) going to be used for building Ruby/Gosu.
# - This is not (yet?) going to replace the Visual Studio project for Windows.
cmake_minimum_required(VERSION 3.10)

project(Gosu VERSION 0.15.2)


##############################################
# Global options

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


##############################################
# Declare dependencies

find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
# Prefer glvnd (the "GL Vendor-Neutral Dispatch library") over plain libGL.so.
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    find_library(CORE_TEXT_LIBRARY CoreText)
    find_library(ICONV_LIBRARY iconv)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
endif()


##############################################
# Create target and set properties

file(GLOB_RECURSE C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/*.c)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Gosu/*.hpp)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Build Gosu as a shared library only for now. This also means we don't have to
# find all of our dependencies again in cmake/GosuConfig.cmake.in, as the
# compiled .so/.dylib file will already reference them.
# TODO: Build both a shared library (mostly for FFI), and a static one.
# See https://stackoverflow.com/q/2152077
add_library(gosu SHARED ${C_FILES} ${HEADER_FILES} ${SOURCE_FILES})

# Ignore deprecation warnings from within Gosu while compiling Gosu itself.
set_target_properties(gosu PROPERTIES
        COMPILE_DEFINITIONS "GOSU_DEPRECATED=")

# Compile all C++ files as Objective-C++ on macOS so that we can use ObjC APIs.
if(APPLE)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES
            COMPILE_FLAGS "-x objective-c++")
endif()

target_include_directories(gosu
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL_sound
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/utf8proc
        ${OPENAL_INCLUDE_DIRS})

target_link_libraries(gosu
    PRIVATE
        SDL2::SDL2
        OpenGL::GL
        ${OPENAL_LIBRARY}
        # Only defined on macOS
        ${COCOA_LIBRARY}
        ${AUDIO_TOOLBOX_LIBRARY}
        ${CORE_TEXT_LIBRARY}
        ${ICONV_LIBRARY}
        # Only defined on Linux
        ${FONTCONFIG_LIBRARIES})


## Set up install paths e.g. for GosuConfig.cmake
#set(CONFIG_FILE_DIR "share/Gosu/cmake")
#set(INSTALL_PKGCONFIG_DIR "share/pkgconfig")
#
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GosuConfig.cmake
#        DESTINATION ${CONFIG_FILE_DIR}
#        COMPONENT development)
#
#set(COMPILING_GOSU 1)
#mark_as_advanced(COMPILING_GOSU)
#
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GosuConfig.cmake)
#
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gosu.pc.in
#        ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
#        @ONLY)
#
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
#        DESTINATION ${INSTALL_PKGCONFIG_DIR}
#        COMPONENT development)
#
#

#include(GNUInstallDirs)
#install(TARGETS Gosu
#        EXPORT gosu-export
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#install(EXPORT gosu-targets
#    FILE
#        cmake/GosuConfig.cmake
#    NAMESPACE
#        Gosu::
#    DESTINATION
#        ${CMAKE_INSTALL_LIBDIR}/cmake/Gosu)


##############################################
# Examples

# Alias our library as Gosu::Gosu so that the examples can depend on it by its
# normal name.
add_library(Gosu::Gosu ALIAS gosu)

# Including the examples here means they will always be built along with Gosu.
# This is fine, we care more about catching errors than compilation speed.
add_subdirectory(examples)
