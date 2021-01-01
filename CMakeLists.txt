# Structure inspired by https://github.com/pabloariasal/modern-cmake-sample
# Assumptions:
# - We cannot go higher than CMake 3.10 while we support Ubuntu 18.04.
# - Prefer static libraries. Users should link Gosu into their games.
# - This is not (yet?) going to be used for building Ruby/Gosu.
# - This is not (yet?) going to replace the Visual Studio project for Windows.
cmake_minimum_required(VERSION 3.10...3.18.4)

project(Gosu VERSION 1.0.0)


##############################################
# Global options

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


##############################################
# Declare dependencies

find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
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
        ${SDL2_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIRS})

target_link_libraries(gosu
    PRIVATE
        # The SDL2::SDL2 target is not available on Ubuntu 18.04.
        ${SDL2_LIBRARIES}
        OpenGL::GL
        ${OPENAL_LIBRARY}
        # Only defined on macOS
        ${COCOA_LIBRARY}
        ${AUDIO_TOOLBOX_LIBRARY}
        ${CORE_TEXT_LIBRARY}
        ${ICONV_LIBRARY}
        # Only defined on Linux
        ${FONTCONFIG_LIBRARIES})


##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Gosu)

install(TARGETS gosu
        EXPORT gosu-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Copied from modern-cmake-sample, although the purpose is not clear to me.
set_target_properties(gosu PROPERTIES EXPORT_NAME Gosu)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export the targets to a script
install(EXPORT gosu-targets
        FILE
        GosuTargets.cmake
        NAMESPACE
        Gosu::
        DESTINATION
        ${INSTALL_CONFIGDIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/GosuConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(EXPORT gosu-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/GosuTargets.cmake
        NAMESPACE Gosu::)


##############################################
# FFI interface, examples, and tests

# Alias our library as Gosu::Gosu so that examples and submodules can depend
# on it by its normal name.
add_library(Gosu::Gosu ALIAS gosu)

# Only build the FFI interface, examples, and tests if this is the top-level CMake module.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(ffi)

    add_subdirectory(examples)

    # If GoogleTest is not available as a CMake module, this is a no-op.
    add_subdirectory(test)
endif()
