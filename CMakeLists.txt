# Assumptions:
# - We cannot go higher than CMake 3.10 while supporting Ubuntu 18.04.
# - Prefer static libraries. Users should link Gosu into their games.
# - This is not (yet?) going to be used for building Ruby/Gosu.
# - This is not (yet?) going to replace the Visual Studio project for Windows.
cmake_minimum_required(VERSION 3.10)

project(Gosu VERSION 0.15.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: Build both a shared library (mostly for FFI) and a static one.
# See https://stackoverflow.com/q/2152077
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

file(GLOB_RECURSE C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/*.c)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Gosu/*.hpp)

# Compile all C++ files as Objective C++ on macOS so we can use Cocoa APIs.
if(APPLE)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS
            "-x objective-c++ -Wno-documentation")
endif()

# Set up install paths e.g. for GosuConfig.cmake
set(CONFIG_FILE_DIR "share/Gosu/cmake")
set(INSTALL_PKGCONFIG_DIR "share/pkgconfig")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GosuConfig.cmake
        DESTINATION ${CONFIG_FILE_DIR}
        COMPONENT development)

set(COMPILING_GOSU 1)
mark_as_advanced(COMPILING_GOSU)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GosuConfig.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gosu.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
        @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR}
        COMPONENT development)

# TODO: Use generator expressions instead of CMAKE_CURRENT_SOURCE_DIR, see:
# https://cliutils.gitlab.io/modern-cmake/chapters/basics/functions.html
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL_sound
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/utf8proc
        ${OPENAL_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS})

add_library(gosu ${SOURCE_FILES} ${C_FILES} ${HEADER_FILES})

set_target_properties(gosu PROPERTIES
        COMPILE_FLAGS "-DGOSU_DEPRECATED="
        INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
        VERSION ${Gosu_VERSION}
        SOVERSION ${Gosu_VERSION_MAJOR}
        )

target_link_libraries(gosu ${GOSU_DEPENDENCIES})

install(TARGETS gosu
        RUNTIME DESTINATION bin COMPONENT library
        LIBRARY DESTINATION lib COMPONENT library
        ARCHIVE DESTINATION lib COMPONENT library)

# Install the header files in the Gosu.
install(DIRECTORY Gosu
        DESTINATION include
        COMPONENT development)
