CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(NetworkPongExample)

#Projects source files
SET(SRC_FILES
	main.cpp
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
LINK_DIRECTORIES(${Gosu_LIBRARY_DIRS})

#Build
ADD_EXECUTABLE(NetworkPongExample ${SRC_FILES})
set_target_properties(NetworkPongExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)

IF(MSVC)
	SET_TARGET_PROPERTIES(NetworkPongExample PROPERTIES COMPILE_FLAGS "/W4 /wd4127")
ELSE(MSVC)
	SET_TARGET_PROPERTIES(NetworkPongExample PROPERTIES COMPILE_FLAGS "-g -std=c++0x")
ENDIF(MSVC)
TARGET_LINK_LIBRARIES(NetworkPongExample ${Gosu_LIBRARIES})
