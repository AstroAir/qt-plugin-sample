cmake_minimum_required(VERSION 3.21)
project(ServicePluginTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Test)

# Find QtPlugin library
find_path(QTPLUGIN_INCLUDE_DIR qtplugin/qtplugin.hpp
    PATHS 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/include
        /usr/local/include
        /usr/include
)

find_library(QTPLUGIN_CORE_LIBRARY 
    NAMES qtplugin-core libqtplugin-core
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/build
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/build
        /usr/local/lib
        /usr/lib
)

if(NOT QTPLUGIN_INCLUDE_DIR OR NOT QTPLUGIN_CORE_LIBRARY)
    message(FATAL_ERROR "QtPlugin library not found. Please build the QtPlugin library first.")
endif()

# Create the service plugin library for testing
add_library(service_plugin_for_tests STATIC
    ../src/service_plugin.hpp
    ../src/service_plugin.cpp
)

target_include_directories(service_plugin_for_tests PUBLIC
    ${QTPLUGIN_INCLUDE_DIR}
    ../src
)

target_link_libraries(service_plugin_for_tests
    Qt6::Core
    ${QTPLUGIN_CORE_LIBRARY}
)

# Enable Qt MOC processing
set_target_properties(service_plugin_for_tests PROPERTIES
    AUTOMOC ON
)

# Create integration test executable
add_executable(ServicePluginIntegrationTests
    test_service_plugin_integration.cpp
)

target_include_directories(ServicePluginIntegrationTests PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
    ../src
)

target_link_libraries(ServicePluginIntegrationTests
    Qt6::Core
    Qt6::Test
    service_plugin_for_tests
    ${QTPLUGIN_CORE_LIBRARY}
)

# Enable Qt MOC processing
set_target_properties(ServicePluginIntegrationTests PROPERTIES
    AUTOMOC ON
)

# Add test to CTest
enable_testing()
add_test(NAME ServicePluginIntegrationTests COMMAND ServicePluginIntegrationTests)

# Create unit test executable for individual components
add_executable(ServicePluginUnitTests
    test_service_plugin_unit.cpp
)

target_include_directories(ServicePluginUnitTests PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
    ../src
)

target_link_libraries(ServicePluginUnitTests
    Qt6::Core
    Qt6::Test
    service_plugin_for_tests
    ${QTPLUGIN_CORE_LIBRARY}
)

set_target_properties(ServicePluginUnitTests PROPERTIES
    AUTOMOC ON
)

add_test(NAME ServicePluginUnitTests COMMAND ServicePluginUnitTests)

# Performance test executable
add_executable(ServicePluginPerformanceTests
    test_service_plugin_performance.cpp
)

target_include_directories(ServicePluginPerformanceTests PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
    ../src
)

target_link_libraries(ServicePluginPerformanceTests
    Qt6::Core
    Qt6::Test
    service_plugin_for_tests
    ${QTPLUGIN_CORE_LIBRARY}
)

set_target_properties(ServicePluginPerformanceTests PROPERTIES
    AUTOMOC ON
)

add_test(NAME ServicePluginPerformanceTests COMMAND ServicePluginPerformanceTests)

# Memory test executable
add_executable(ServicePluginMemoryTests
    test_service_plugin_memory.cpp
)

target_include_directories(ServicePluginMemoryTests PRIVATE
    ${QTPLUGIN_INCLUDE_DIR}
    ../src
)

target_link_libraries(ServicePluginMemoryTests
    Qt6::Core
    Qt6::Test
    service_plugin_for_tests
    ${QTPLUGIN_CORE_LIBRARY}
)

set_target_properties(ServicePluginMemoryTests PROPERTIES
    AUTOMOC ON
)

add_test(NAME ServicePluginMemoryTests COMMAND ServicePluginMemoryTests)

# Print configuration summary
message(STATUS "=== Service Plugin Tests Configuration ===")
message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "QtPlugin include dir: ${QTPLUGIN_INCLUDE_DIR}")
message(STATUS "QtPlugin library: ${QTPLUGIN_CORE_LIBRARY}")
message(STATUS "Test executables:")
message(STATUS "  - ServicePluginIntegrationTests")
message(STATUS "  - ServicePluginUnitTests") 
message(STATUS "  - ServicePluginPerformanceTests")
message(STATUS "  - ServicePluginMemoryTests")
message(STATUS "==========================================")

# Custom target to run all tests
add_custom_target(run_service_plugin_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS 
        ServicePluginIntegrationTests
        ServicePluginUnitTests
        ServicePluginPerformanceTests
        ServicePluginMemoryTests
    COMMENT "Running all service plugin tests"
)
