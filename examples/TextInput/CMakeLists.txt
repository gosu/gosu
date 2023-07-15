cmake_minimum_required(VERSION 3.22.1)

project(TextInput)

add_executable(TextInputExample
        TextInput.cpp)

# Gosu's own examples have to be built into the parent directory so that they
# can find the shared 'media' directory.
# You do not have to do this in your own Gosu projects.
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH CACHE)
set_target_properties(TextInputExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PARENT_DIR})

if (NOT TARGET Gosu::Gosu)
    find_package(Gosu REQUIRED)
endif ()
target_link_libraries(TextInputExample Gosu::Gosu)
