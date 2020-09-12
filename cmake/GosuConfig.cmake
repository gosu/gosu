# Find Gosu includes and library
#
# This module defines
#  GOSU_INCLUDE_DIRS
#  GOSU_LIBRARY, the Gosu library name to link against.
#  GOSU_DEPENDENCIES, the libraries that a static Gosu library needs to be linked against.
#  GOSU_FOUND, If false, do not try to use Gosu
#

include(FindPackageMessage)

get_filename_component(THIS_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(PREFIX ${THIS_DIR}/../../..)

set(GOSU_INCLUDE_DIR "${PREFIX}/include")
set(GOSU_INCLUDE_DIRS ${GOSU_INCLUDE_DIR})

find_package(OpenAL REQUIRED)
# Prefer glvnd (the "GL Vendor-Neutral Dispatch library") over plain libGL.so
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

set(GOSU_DEPENDENCIES
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARY}
    ${OPENAL_LIBRARY}
)

link_directories(
    ${SDL2_LIBRARY_DIRS}
)

if(NOT APPLE)
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

    set(GOSU_DEPENDENCIES ${GOSU_DEPENDENCIES}
        ${FONTCONFIG_LIBRARIES}
    )

    link_directories(
        ${FONTCONFIG_LIBRARY_DIRS}
    )
endif()

if(APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CARBON_LIBRARY Carbon)
    find_library(APPLICATION_SERVICES_LIBRARY ApplicationServices)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    find_library(ICONV_LIBRARY iconv)
    
    mark_as_advanced(
        FOUNDATION_LIBRARY
        COCOA_LIBRARY
        CARBON_LIBRARY
        APPLICATION_SERVICES_LIBRARY
        AUDIO_TOOLBOX_LIBRARY
        ICONV_LIBRARY
    )
    
    set(GOSU_DEPENDENCIES ${GOSU_DEPENDENCIES}
        ${FOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
        ${CARBON_LIBRARY}
        ${APPLICATION_SERVICES_LIBRARY}
        ${AUDIO_TOOLBOX_LIBRARY}
        ${ICONV_LIBRARY}
    )
endif()

if(NOT COMPILING_GOSU)
    find_library(GOSU_LIBRARY NAMES gosu libgosu)
    set(GOSU_LIBRARIES "${GOSU_LIBRARY}")
    find_package_handle_standard_args(Gosu DEFAULT_MSG GOSU_LIBRARY GOSU_INCLUDE_DIR)
    mark_as_advanced(GOSU_LIBRARY GOSU_INCLUDE_DIR)
endif()
