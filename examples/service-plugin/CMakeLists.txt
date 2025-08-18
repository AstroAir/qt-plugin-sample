cmake_minimum_required(VERSION 3.21)
project(ServicePluginExample VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)

# Find QtPlugin library
find_path(QTPLUGIN_INCLUDE_DIR qtplugin/qtplugin.hpp
    PATHS 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/include
        /usr/local/include
        /usr/include
)

find_library(QTPLUGIN_CORE_LIBRARY 
    NAMES qtplugin-core libqtplugin-core
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/build
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/build
        /usr/local/lib
        /usr/lib
)

if(NOT QTPLUGIN_INCLUDE_DIR OR NOT QTPLUGIN_CORE_LIBRARY)
    message(FATAL_ERROR "QtPlugin library not found. Please build the QtPlugin library first.")
endif()

# Create the service plugin
add_library(service_plugin SHARED
    src/service_plugin.hpp
    src/service_plugin.cpp
)

target_include_directories(service_plugin PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
    src
)

target_link_libraries(service_plugin
    Qt6::Core
    ${QTPLUGIN_CORE_LIBRARY}
)

# Set plugin properties
set_target_properties(service_plugin PROPERTIES
    OUTPUT_NAME "service_plugin"
    SUFFIX ".qtplugin"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Enable Qt MOC processing
set_target_properties(service_plugin PROPERTIES
    AUTOMOC ON
)

# Create metadata file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata.json
    ${CMAKE_BINARY_DIR}/plugins/metadata.json
    COPYONLY
)

# Create test application
add_executable(ServicePluginExample
    src/main.cpp
)

target_include_directories(ServicePluginExample PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
)

target_link_libraries(ServicePluginExample
    Qt6::Core
    ${QTPLUGIN_CORE_LIBRARY}
)

# Copy plugin to test application directory
add_custom_command(TARGET ServicePluginExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ServicePluginExample>/plugins
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        $<TARGET_FILE:service_plugin> 
        $<TARGET_FILE_DIR:ServicePluginExample>/plugins/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_BINARY_DIR}/plugins/metadata.json
        $<TARGET_FILE_DIR:ServicePluginExample>/plugins/
)

# Optional: Create unit tests if testing is enabled
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    add_executable(ServicePluginTests
        tests/test_service_plugin.cpp
        src/service_plugin.hpp
        src/service_plugin.cpp
    )
    
    target_include_directories(ServicePluginTests PRIVATE
        ${QTPLUGIN_INCLUDE_DIR}
        src
    )
    
    target_link_libraries(ServicePluginTests
        Qt6::Core
        Qt6::Test
        ${QTPLUGIN_CORE_LIBRARY}
    )
    
    set_target_properties(ServicePluginTests PROPERTIES
        AUTOMOC ON
    )
    
    # Add test to CTest
    enable_testing()
    add_test(NAME ServicePluginTests COMMAND ServicePluginTests)
endif()

# Installation rules
install(TARGETS service_plugin
    LIBRARY DESTINATION plugins
    RUNTIME DESTINATION plugins
)

install(FILES metadata.json
    DESTINATION plugins
)

# Print configuration summary
message(STATUS "=== Service Plugin Example Configuration ===")
message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "QtPlugin include dir: ${QTPLUGIN_INCLUDE_DIR}")
message(STATUS "QtPlugin library: ${QTPLUGIN_CORE_LIBRARY}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Plugin output: ${CMAKE_BINARY_DIR}/plugins/service_plugin.qtplugin")
message(STATUS "=============================================")
