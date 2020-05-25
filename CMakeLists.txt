# Assumptions:
# - We cannot go higher than CMake 3.10 while supporting Ubuntu 18.04.
# - Prefer static libraries. Users should link Gosu into their games.
# - This is not (yet?) going to be used for building Ruby/Gosu.
# - This is not (yet?) going to replace the Visual Studio project for Windows.
cmake_minimum_required(VERSION 3.10)

project(Gosu VERSION 0.15.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()

file(GLOB C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Gosu/*.hpp)

# Compile all C++ files as Objective C++ on macOS so we can use Cocoa APIs.
if(APPLE)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS
            "-x objective-c++ -Wno-documentation")
endif()

# Set up install paths e.g. for GosuConfig.cmake
if(WIN32)
    set(CONFIG_FILE_DIR "CMake")
    set(GOSU_LIBRARY_INSTALL_DIRS "Gosu/lib")
    set(GOSU_INCLUDE_INSTALL_DIRS "Gosu/include")
else()
    set(CONFIG_FILE_DIR "share/Gosu/cmake")
    set(GOSU_LIBRARY_INSTALL_DIRS "lib")
    set(GOSU_INCLUDE_INSTALL_DIRS "include/Gosu")
    set(INSTALL_PKGCONFIG_DIR "share/pkgconfig")
endif()


configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GosuConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/GosuConfig.cmake
        @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/GosuConfig.cmake
        DESTINATION ${CONFIG_FILE_DIR}
        COMPONENT development)

set(COMPILING_GOSU 1)
mark_as_advanced(COMPILING_GOSU)

include(${CMAKE_CURRENT_BINARY_DIR}/cmake/GosuConfig.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gosu.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
        @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/gosu.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR}
        COMPONENT development)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OPENAL_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
)

# "Sources" and "Headers" are the group names in Visual Studio.
source_group("Sources" FILES ${SOURCE_FILES} ${C_FILES})
source_group("Headers" FILES ${HEADER_FILES})

add_library(gosu ${SOURCE_FILES} ${C_FILES} ${HEADER_FILES})

set_target_properties(gosu PROPERTIES
        COMPILE_FLAGS "-DGOSU_DEPRECATED="
        INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
        VERSION ${GOSU_VERSION}
        SOVERSION ${GOSU_VERSION_MAJOR}
        )

if(MSVC)
    # Disable warning on STL exports
    set_target_properties(gosu PROPERTIES COMPILE_FLAGS "/W4 /wd4251 /wd4127")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif()

target_link_libraries(gosu ${GOSU_DEPENDENCIES})

if(WIN32)
    install(TARGETS gosu
            RUNTIME DESTINATION Gosu/bin COMPONENT library
            LIBRARY DESTINATION Gosu/lib COMPONENT library
            ARCHIVE DESTINATION Gosu/lib COMPONENT library)

    # Install the core header files in the Gosu subfolder.
    install(DIRECTORY Gosu
            DESTINATION Gosu
            COMPONENT development)

    # On Windows, we also install the debug information. It's unfortunate that
    # we have to hard-code the 'Debug' part of the path, but CMake doesn't seem
    # to provide a way around this. The best I found was this (and it's a hack):
    # http://www.cmake.org/pipermail/cmake/2007-October/016924.html
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/Gosu.pdb
            DESTINATION Gosu/lib
            CONFIGURATIONS Debug)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/Gosu.pdb
            DESTINATION Gosu/lib
            CONFIGURATIONS RelWithDebInfo)
else()
    install(TARGETS gosu
            RUNTIME DESTINATION bin COMPONENT library
            LIBRARY DESTINATION lib COMPONENT library
            ARCHIVE DESTINATION lib COMPONENT library)

    # Install the header files in the Gosu.
    install(DIRECTORY Gosu
            DESTINATION include
            COMPONENT development)
endif()
