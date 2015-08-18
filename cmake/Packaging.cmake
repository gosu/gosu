set(CPACK_PACKAGE_NAME "libgosu-dev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "2D game development library")
set(CPACK_PACKAGE_VENDOR "Julian Raschke and Gosu Contributors")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR ${GOSU_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GOSU_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GOSU_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Gosu-${GOSU_VERSION}")
set(CPACK_PACKAGE_CONTACT "Oliver Schneider <mail@oli-obk.de>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
# Taken from https://github.com/gosu/gosu/wiki/Getting-Started-on-Linux
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libsdl2-dev, libsdl2-ttf-dev, libpango1.0-dev, libgl1-mesa-dev, libopenal-dev, libsndfile-dev")

if(WIN32 AND NOT UNIX)
  # There is a bug in NSIS that does not handle full unix paths properly.
  # Make sure there is at least one set of four backslashes.
  set(CPACK_NSIS_DISPLAY_NAME "Gosu ${GOSU_VERSION}")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\libgosu.org/")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\libgosu.org")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)

cpack_add_component(library
    DISPLAY_NAME "Library"
    DESCRIPTION "The runtime libraries"
    REQUIRED)
cpack_add_component(development
    DISPLAY_NAME "Development"
    DESCRIPTION "Files required for developing with Gosu"
    DEPENDS library)
