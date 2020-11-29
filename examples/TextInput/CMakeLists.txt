cmake_minimum_required(VERSION 3.10)

project(TextInput)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TextInputExample
        TextInput.cpp)

# Gosu's own examples have to be built into the parent directory so that they
# can find the shared 'media' directory.
# You do not have to do this in your own Gosu projects.
set_target_properties(TextInputExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

if(NOT TARGET Gosu::Gosu)
    find_package(Gosu REQUIRED)
endif()
target_link_libraries(TextInputExample Gosu::Gosu)
