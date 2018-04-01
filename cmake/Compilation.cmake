file(GLOB C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.c)
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../Gosu/*.hpp)

# Compile all C++ files as Objective C++ on macOS.
if(APPLE)
    set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -Wno-documentation")
endif()

# TODO - this should probably go into Installation.cmake?
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

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
    ${CMAKE_CURRENT_SOURCE_DIR}/GosuConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake
    @ONLY)
    
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake
    DESTINATION ${CONFIG_FILE_DIR}
    COMPONENT development)

set(COMPILING_GOSU 1)
mark_as_advanced(COMPILING_GOSU)

include(${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gosu.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/gosu.pc
    @ONLY)
    
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gosu.pc
    DESTINATION ${INSTALL_PKGCONFIG_DIR}
    COMPONENT development)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${FREETYPE_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
    ${PANGOFT_INCLUDE_DIRS}
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
