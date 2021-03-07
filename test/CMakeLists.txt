find_package(GTest)

if(GTest_FOUND)
    enable_testing()

    file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    add_executable(GosuTests ${TEST_FILES})
    target_link_libraries(GosuTests Gosu::Gosu gosu-ffi GTest::gtest GTest::gtest_main)

    # Run the tests from within the test/ subdirectory so that we can load
    # media files using relative paths.
    gtest_discover_tests(GosuTests WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

    # When CLion automatically detects our tests, it does not respect the
    # WORKING_DIRECTORY option above; change the output directory to the same
    # directory as a workaround.
    set_target_properties(GosuTests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()
