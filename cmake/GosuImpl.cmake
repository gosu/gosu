###############################################
#
#Copyright (c) 2005-2012 Matt Williams <matt@volumesoffun.com>
#Copyright (c) 2005-2012 David Williams <david@volumesoffun.com>
#Copyright (c) 2012 Oliver Schneider <mail@oli-obk.de>
#
#This software is provided 'as-is', without any express or implied
#warranty. In no event will the authors be held liable for any damages
#arising from the use of this software.
#
#Permission is granted to anyone to use this software for any purpose,
#including commercial applications, and to alter it and redistribute it
#freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
# claim that you wrote the original software. If you use this software
# in a product, an acknowledgment in the product documentation would be
# appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must not be
# misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
# distribution.
#
###############################################

#By default only build static libraries
#If we ever require CMake 2.8 then use http://www.kitware.com/blog/home/post/82
option(BUILD_STATIC_LIBRARIES "Build static libraries" ON)
if(WIN32)
	option(BUILD_DYNAMIC_LIBRARIES "Build dynamic libraries" OFF)
else()
    OPTION(BUILD_DYNAMIC_LIBRARIES "Should the dynamic libraries be built? WARNING: Enable at your own risk" OFF)
endif()

OPTION(GOSU_CPP11_ENABLED "Should Gosu's c++11 features get enabled? (this is probably only relevant on Linux)" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


if(WIN32)
	#If both are enabled then disable the dyanamic build
	if(BUILD_STATIC_LIBRARIES AND BUILD_DYNAMIC_LIBRARIES)
		message(STATUS "Building both static and dynamic libraries is not supported on Windows. Disabling dynamic libraries.")
		set(BUILD_DYNAMIC_LIBRARIES OFF CACHE BOOL "Build dynamic libraries" FORCE)
	endif()
	#If both are diabled then re-enable the static build
	if(NOT BUILD_STATIC_LIBRARIES AND NOT BUILD_DYNAMIC_LIBRARIES)
		message(STATUS "Both dynamic and static libraries were disabled - re-enabling static build.")
		set(BUILD_STATIC_LIBRARIES ON CACHE BOOL "Build static libraries" FORCE)
	endif()
else()
	#It's nonsense to disable both so on Linux, re-enable both.
	if(NOT BUILD_STATIC_LIBRARIES AND NOT BUILD_DYNAMIC_LIBRARIES)
		message(STATUS "Both dynamic and static libraries were disabled - re-enabling both.")
		set(BUILD_STATIC_LIBRARIES ON CACHE BOOL "Build static libraries" FORCE)
		set(BUILD_DYNAMIC_LIBRARIES ON CACHE BOOL "Build dynamic libraries" FORCE)
	endif()
endif()

#Projects source files
SET(CORE_SRC_FILES
    Inspection.cpp
    IO.cpp
    Math.cpp
    Graphics/BitmapBMP.cpp
    Graphics/BitmapColorKey.cpp
    Graphics/Bitmap.cpp
    Graphics/BitmapFreeImage.cpp
    Graphics/BitmapUtils.cpp
    Graphics/BlockAllocator.cpp
    Graphics/Color.cpp
    Graphics/Font.cpp
    Graphics/Graphics.cpp
    Graphics/Image.cpp
    Graphics/LargeImageData.cpp
    Graphics/Resolution.cpp
    Graphics/TexChunk.cpp
    Graphics/Texture.cpp
    Graphics/Transform.cpp
    Sockets/CommSocket.cpp
    Sockets/ListenerSocket.cpp
    Sockets/MessageSocket.cpp
    Sockets/Socket.cpp
    Audio/AudioOpenAL.cpp
)

if(WIN32)
    set(CORE_SRC_FILES ${CORE_SRC_FILES}
    Graphics/TextWin.cpp
    Graphics/TextTTFWin.cpp
    Graphics/BitmapGDIplus.cpp
    WinMain.cpp
    WinUtility.cpp
    TimingWin.cpp
    WindowWin.cpp
    UtilityWin.cpp
    FileWin.cpp
    InputWin.cpp
    TextInputWin.cpp
    DirectoriesWin.cpp
    )
elseif(APPLE)
    set(CORE_SRC_FILES ${CORE_SRC_FILES}
    Graphics/ResolutionApple.mm
    Graphics/TextMac.cpp
    TimingApple.cpp
    )
else()
    set(CORE_SRC_FILES ${CORE_SRC_FILES}
    Graphics/TextUnix.cpp
    TimingUnix.cpp
    )
endif()

if(NOT WIN32)
    set(CORE_SRC_FILES ${CORE_SRC_FILES}
    DirectoriesUnix.cpp
    FileUnix.cpp
    InputX.cpp
    TextInputX.cpp
    WindowX.cpp
    Utility.cpp
    Graphics/Text.cpp
    )
endif()

set(LOOP_ITEMS ${CORE_SRC_FILES})
set(CORE_SRC_FILES "")
foreach(item ${LOOP_ITEMS})
    set(CORE_SRC_FILES ${CORE_SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/../GosuImpl/${item})
endforeach()

#Projects headers files
SET(CORE_INC_FILES
    ../Gosu/Directories.hpp
    ../Gosu/Input.hpp
    ../Gosu/Timing.hpp
    ../Gosu/Audio.hpp
    ../Gosu/Font.hpp
    ../Gosu/Inspection.hpp
    ../Gosu/TR1.hpp
    ../Gosu/AutoLink.hpp
    ../Gosu/Fwd.hpp
    ../Gosu/IO.hpp
    ../Gosu/Bitmap.hpp
    ../Gosu/Gosu.hpp
    ../Gosu/Math.hpp
    ../Gosu/Version.hpp
    ../Gosu/GraphicsBase.hpp
    ../Gosu/Platform.hpp
    ../Gosu/Window.hpp
    ../Gosu/Graphics.hpp
    ../Gosu/Sockets.hpp
    ../Gosu/ImageData.hpp
    ../Gosu/Text.hpp
    ../Gosu/Color.hpp
    ../Gosu/Image.hpp
    ../Gosu/TextInput.hpp
    ../Gosu/Buttons.hpp
    ../Gosu/Utility.hpp
)

if(WIN32)
SET(CORE_INC_FILES ${CORE_INC_FILES}
    ../Gosu/WinUtility.hpp
)
endif()


#Set up install paths e.g. for GosuConfig.cmake
if(WIN32)
	set(CONFIG_FILE_DIR "CMake")
	set(Gosu_LIBRARY_INSTALL_DIRS "Gosu/lib")
	set(Gosu_INCLUDE_INSTALL_DIRS "Gosu/include")
else(WIN32)
	set(CONFIG_FILE_DIR "share/Gosu/cmake")
	set(Gosu_LIBRARY_INSTALL_DIRS "lib")
	set(Gosu_INCLUDE_INSTALL_DIRS "include/Gosu")
    set(INSTALL_PKGCONFIG_DIR "share/pkgconfig")
endif(WIN32)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/GosuConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake DESTINATION ${CONFIG_FILE_DIR} COMPONENT development)

include(${CMAKE_CURRENT_BINARY_DIR}/GosuConfig.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gosu.pc.in ${CMAKE_CURRENT_BINARY_DIR}/gosu.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gosu.pc DESTINATION ${INSTALL_PKGCONFIG_DIR} COMPONENT development)

#Tell CMake the paths
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${FREETYPE_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
    ${SDL_TTF_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
    ${PANGOFT_INCLUDE_DIRS}
    ${XINERAMA_INCLUDE_DIRS}
    ${FREEIMAGE_INCLUDE_DIRS}
    ${VORBIS_INCLUDE_DIRS}
)

#NOTE: The following line should be uncommented when building shared libs.

#"Sources" and "Headers" are the group names in Visual Studio.
#They may have other uses too...
SOURCE_GROUP("Sources" FILES ${CORE_SRC_FILES})
SOURCE_GROUP("Headers" FILES ${CORE_INC_FILES})

#Core
#Build
IF(BUILD_STATIC_LIBRARIES)
	ADD_LIBRARY(GosuStatic STATIC ${CORE_SRC_FILES} ${CORE_INC_FILES})
	SET_TARGET_PROPERTIES(GosuStatic PROPERTIES OUTPUT_NAME "Gosu")
	SET_TARGET_PROPERTIES(GosuStatic PROPERTIES VERSION ${GOSU_VERSION} SOVERSION ${GOSU_VERSION_MAJOR})
	IF(MSVC)
			SET_TARGET_PROPERTIES(GosuStatic PROPERTIES COMPILE_FLAGS "/W4 /wd4251 /wd4127") #Disable warning on STL exports
	ENDIF(MSVC)
	SET(Gosu_LIBRARY "GosuStatic")
ENDIF()
IF(BUILD_DYNAMIC_LIBRARIES)
	ADD_LIBRARY(GosuDynamic SHARED ${CORE_SRC_FILES} ${CORE_INC_FILES})
	SET_TARGET_PROPERTIES(GosuDynamic PROPERTIES OUTPUT_NAME "Gosu")
	SET_TARGET_PROPERTIES(GosuDynamic PROPERTIES COMPILE_FLAGS "-DGOSU_SHARED_EXPORTS")
	SET_TARGET_PROPERTIES(GosuDynamic PROPERTIES VERSION ${GOSU_VERSION} SOVERSION ${GOSU_VERSION_MAJOR})
	IF(MSVC)
			SET_TARGET_PROPERTIES(GosuDynamic PROPERTIES COMPILE_FLAGS "/W4 /wd4251 /wd4127") #Disable warning on STL exports
	ENDIF(MSVC)
	# out of SOME reason, we cannot link to gl in the executable
    find_package(OpenGL REQUIRED)
	target_link_libraries(GosuDynamic ${OPENGL_LIBRARY})
	SET(Gosu_LIBRARY "GosuDynamic")
ENDIF()

#Install
IF(WIN32)
	IF(BUILD_STATIC_LIBRARIES)
		INSTALL(TARGETS GosuStatic
			RUNTIME DESTINATION Gosu/bin COMPONENT library
			LIBRARY DESTINATION Gosu/lib COMPONENT library
			ARCHIVE DESTINATION Gosu/lib COMPONENT library
		)
	ENDIF()
	IF(BUILD_DYNAMIC_LIBRARIES)
		INSTALL(TARGETS GosuDynamic
			RUNTIME DESTINATION Gosu/bin COMPONENT library
			LIBRARY DESTINATION Gosu/lib COMPONENT library
			ARCHIVE DESTINATION Gosu/lib COMPONENT library
		)
	ENDIF()

	#Install the core header files, including the ones in the Gosu subfolder.
	INSTALL(DIRECTORY ../Gosu DESTINATION Gosu COMPONENT development PATTERN "*.svn*" EXCLUDE)
	
	#On windows, we also install the debug information. It's unfortunate that we have to hard-code
	#the 'Debug' part of the path, but CMake doesn't seem to provide a way around this. The best I
	#found was: http://www.cmake.org/pipermail/cmake/2007-October/016924.html (and it is a bit ugly).
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/Gosu.pdb DESTINATION Gosu/lib CONFIGURATIONS Debug)
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/Gosu.pdb DESTINATION Gosu/lib CONFIGURATIONS RelWithDebInfo)
ELSE(WIN32)
	IF(BUILD_STATIC_LIBRARIES)
		INSTALL(TARGETS GosuStatic
			RUNTIME DESTINATION bin COMPONENT library
			LIBRARY DESTINATION lib COMPONENT library
			ARCHIVE DESTINATION lib COMPONENT library
		)
	ENDIF()
	IF(BUILD_DYNAMIC_LIBRARIES)
		INSTALL(TARGETS GosuDynamic
			RUNTIME DESTINATION bin COMPONENT library
			LIBRARY DESTINATION lib COMPONENT library
			ARCHIVE DESTINATION lib COMPONENT library
		)
	ENDIF()
	
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -pedantic")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")
	if(GOSU_CPP11_ENABLED)
	    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	endif()

	#Install the core header files, including the ones in the Gosu subfolder.
	#this also installs windows headers, TODO: fix?
	INSTALL(DIRECTORY ../Gosu DESTINATION include COMPONENT development PATTERN "*.svn*" EXCLUDE PATTERN "*~" EXCLUDE)
ENDIF(WIN32)


