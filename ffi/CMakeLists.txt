file(GLOB FFI_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB FFI_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(gosu-ffi SHARED ${FFI_HEADER_FILES} ${FFI_SOURCE_FILES})

# Ignore deprecation warnings from within Gosu while compiling Gosu itself.
set_target_properties(gosu-ffi PROPERTIES
        COMPILE_DEFINITIONS "GOSU_DEPRECATED=;GOSU_FFI_EXPORTS")

target_link_libraries(gosu-ffi
    PRIVATE
        Gosu::Gosu)

include(GNUInstallDirs)

install(TARGETS gosu-ffi
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
