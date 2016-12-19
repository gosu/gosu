set(SOURCE_FILES
    ../src/Bitmap.cpp
    ../src/BitmapIO.cpp
    ../src/BlockAllocator.cpp
    ../src/Color.cpp
    ../src/Font.cpp
    ../src/Graphics.cpp
    ../src/Image.cpp
    ../src/Input.cpp
    ../src/Inspection.cpp
    ../src/IO.cpp
    ../src/LargeImageData.cpp
    ../src/Macro.cpp
    ../src/Math.cpp
    ../src/Resolution.cpp
    ../src/TexChunk.cpp
    ../src/Text.cpp
    ../src/TextInput.cpp
    ../src/Texture.cpp
    ../src/Transform.cpp
    ../src/Utility.cpp
    ../src/Window.cpp
    ../src/stb_vorbis.c
)

if(WIN32)
    set(SOURCE_FILES ${SOURCE_FILES}
        ../src/Audio.cpp
        ../src/DirectoriesWin.cpp
        ../src/FileWin.cpp
        ../src/InputWin.cpp
        ../src/TextTTFWin.cpp
        ../src/TextWin.cpp
        ../src/TimingWin.cpp
        ../src/UtilityWin.cpp
        ../src/WinUtility.cpp
    )
elseif(APPLE)
    set(SOURCE_FILES ${SOURCE_FILES}
        ../src/Audio.mm
        ../src/DirectoriesApple.mm
        ../src/FileUnix.cpp
        ../src/ResolutionApple.mm
        ../src/TextApple.mm
        ../src/TimingApple.cpp
        ../src/UtilityApple.mm
    )
else()
    set(SOURCE_FILES ${SOURCE_FILES}
        ../src/Audio.cpp
        ../src/DirectoriesUnix.cpp
        ../src/FileUnix.cpp
        ../src/TextUnix.cpp
        ../src/TimingUnix.cpp
    )
endif()

set(HEADER_FILES
    ../Gosu/Audio.hpp
    ../Gosu/AutoLink.hpp
    ../Gosu/Bitmap.hpp
    ../Gosu/Buttons.hpp
    ../Gosu/Color.hpp
    ../Gosu/Directories.hpp
    ../Gosu/Font.hpp
    ../Gosu/Fwd.hpp
    ../Gosu/Gosu.hpp
    ../Gosu/Graphics.hpp
    ../Gosu/GraphicsBase.hpp
    ../Gosu/Image.hpp
    ../Gosu/ImageData.hpp
    ../Gosu/Input.hpp
    ../Gosu/Inspection.hpp
    ../Gosu/IO.hpp
    ../Gosu/Math.hpp
    ../Gosu/Platform.hpp
    ../Gosu/Text.hpp
    ../Gosu/TextInput.hpp
    ../Gosu/Timing.hpp
    ../Gosu/Utility.hpp
    ../Gosu/Version.hpp
    ../Gosu/Window.hpp
)

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
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
    ${PANGOFT_INCLUDE_DIRS}
)

# "Sources" and "Headers" are the group names in Visual Studio.
source_group("Sources" FILES ${SOURCE_FILES})
source_group("Headers" FILES ${HEADER_FILES})

add_library(gosu ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(gosu PROPERTIES
    COMPILE_FLAGS "-DGOSU_SHARED_EXPORTS"
    INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}
    VERSION ${GOSU_VERSION}
    SOVERSION ${GOSU_VERSION_MAJOR}
)

if(MSVC)
    # Disable warning on STL exports
    set_target_properties(gosu PROPERTIES COMPILE_FLAGS "/W4 /wd4251 /wd4127")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

target_link_libraries(gosu ${GOSU_DEPENDENCIES})
