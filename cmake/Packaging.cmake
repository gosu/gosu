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

#INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "libgosu-dev")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "a 2D game development library")
SET(CPACK_PACKAGE_VENDOR "unknown")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR ${GOSU_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${GOSU_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${GOSU_VERSION_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Gosu${GOSU_VERSION}")
SET(CPACK_PACKAGE_CONTACT "Oliver Schneider <mail@oli-obk.de>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
# TODO - outdated!
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "build-essential, freeglut3-dev, libfreeimage-dev, libgl1-mesa-dev, libopenal-dev, libpango1.0-dev, libsdl-mixer1.2-dev, libsdl-ttf2.0-dev, libsndfile-dev, libxinerama-dev")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSIS that does not handle full unix paths properly.
  # Make sure there is at least one set of four backslashes.
  SET(CPACK_NSIS_DISPLAY_NAME "Gosu ${GOSU_VERSION}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\libgosu.org/")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\libgosu.org")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  #SET(CPACK_STRIP_FILES "bin/MyExecutable")
  #SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)

CPACK_ADD_COMPONENT(library DISPLAY_NAME "Library" DESCRIPTION "The runtime libraries" REQUIRED)
CPACK_ADD_COMPONENT(development DISPLAY_NAME "Development" DESCRIPTION "Files required for developing with Gosu" DEPENDS library)
cpack_add_component_group(bindings DISPLAY_NAME "Bindings" DESCRIPTION "Language bindings")
