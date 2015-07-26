if(WIN32)
    install(TARGETS gosu
        RUNTIME DESTINATION Gosu/bin COMPONENT library
        LIBRARY DESTINATION Gosu/lib COMPONENT library
        ARCHIVE DESTINATION Gosu/lib COMPONENT library)

    # Install the core header files in the Gosu subfolder.
    install(DIRECTORY ../Gosu
        DESTINATION Gosu
        COMPONENT development)
    
    # On windows, we also install the debug information. It's unfortunate that we have to hard-code
    # the 'Debug' part of the path, but CMake doesn't seem to provide a way around this. The best I
    # found was: http://www.cmake.org/pipermail/cmake/2007-October/016924.html (and it is a bit ugly).
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/Gosu.pdb
        DESTINATION Gosu/lib
        CONFIGURATIONS Debug)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/Gosu.pdb
        DESTINATION Gosu/lib
        CONFIGURATIONS RelWithDebInfo)
else(WIN32)
    install(TARGETS gosu
        RUNTIME DESTINATION bin COMPONENT library
        LIBRARY DESTINATION lib COMPONENT library
        ARCHIVE DESTINATION lib COMPONENT library)
    
    # Install the header files in the Gosu.
    install(DIRECTORY ../Gosu
        DESTINATION include
        COMPONENT development)
endif(WIN32)

