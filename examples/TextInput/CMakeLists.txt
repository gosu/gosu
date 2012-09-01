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

###############################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(TextInputExample)

#Projects source files
SET(SRC_FILES
	TextInput.cpp
	)

#Projects headers files	
SET(INC_FILES
	)

#"Sources" and "Headers" are the group names in Visual Studio.
#They may have other uses too...
SOURCE_GROUP("Sources" FILES ${SRC_FILES})
SOURCE_GROUP("Headers" FILES ${INC_FILES})

find_package(Gosu REQUIRED)

INCLUDE_DIRECTORIES(${Gosu_INCLUDE_DIRS})
# we link to the binary dir as gosu might not be installed
LINK_DIRECTORIES(${Gosu_LIBRARY_DIRS} ${Gosu_BINARY_DIR})

#Build
ADD_EXECUTABLE(TextInputExample ${SRC_FILES})
set_target_properties(TextInputExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)

IF(MSVC)
	SET_TARGET_PROPERTIES(TextInputExample PROPERTIES COMPILE_FLAGS "/W4 /wd4127")
ENDIF(MSVC)
TARGET_LINK_LIBRARIES(TextInputExample ${Gosu_LIBRARIES})

#Install - Only install the example in Windows
IF(WIN32)
	INSTALL(TARGETS TextInputExample
		RUNTIME DESTINATION Examples/TextInput/bin
		LIBRARY DESTINATION Examples/TextInput/lib
		ARCHIVE DESTINATION Examples/TextInput/lib
		COMPONENT example
	)
ENDIF(WIN32)
