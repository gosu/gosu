cmake_minimum_required(VERSION 3.22.1)

project(Tutorial)

add_executable(TutorialExample
        Tutorial.cpp)

# Gosu's own examples have to be built into the parent directory so that they
# can find the shared 'media' directory.
# You do not have to do this in your own Gosu projects.
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH CACHE)
set_target_properties(TutorialExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PARENT_DIR})

if (NOT TARGET Gosu::Gosu)
    find_package(Gosu REQUIRED)
endif ()
target_link_libraries(TutorialExample Gosu::Gosu)
