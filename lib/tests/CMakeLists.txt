# QtPlugin Library Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Find Qt6 Test component
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Sql)

# Enable automatic MOC
set(CMAKE_AUTOMOC ON)

# Configuration Manager Tests
add_executable(test_configuration_manager
    test_configuration_manager.cpp
)

target_include_directories(test_configuration_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_configuration_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

# Add test to CTest
add_test(NAME ConfigurationManagerTests COMMAND test_configuration_manager)

# Set test properties
set_tests_properties(ConfigurationManagerTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test executable (optional)
install(TARGETS test_configuration_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Resource Management Tests
add_executable(test_resource_management
    test_resource_management.cpp
)

target_include_directories(test_resource_management PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_resource_management PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    QtPluginCore
)

# Add test to CTest
add_test(NAME ResourceManagementTests COMMAND test_resource_management)

# Set test properties
set_tests_properties(ResourceManagementTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test executable (optional)
install(TARGETS test_resource_management
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Plugin Manager Tests
add_executable(test_plugin_manager
    test_plugin_manager.cpp
)

target_include_directories(test_plugin_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME PluginManagerTests COMMAND test_plugin_manager)
set_tests_properties(PluginManagerTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_plugin_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Security Manager Tests
add_executable(test_security_manager
    test_security_manager.cpp
)

target_include_directories(test_security_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_security_manager PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
    QtPluginSecurity
)

add_test(NAME SecurityManagerTests COMMAND test_security_manager)
set_tests_properties(SecurityManagerTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_security_manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Message Bus Tests
add_executable(test_message_bus
    test_message_bus_simple.cpp
)

target_include_directories(test_message_bus PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_message_bus PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME MessageBusTests COMMAND test_message_bus)
set_tests_properties(MessageBusTests PROPERTIES
    TIMEOUT 120
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_message_bus
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Version Tests
add_executable(test_version
    test_version_simple.cpp
)

target_include_directories(test_version PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_version PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME VersionTests COMMAND test_version)
set_tests_properties(VersionTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_version
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Error Handling Tests
add_executable(test_error_handling
    test_error_handling.cpp
)

target_include_directories(test_error_handling PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_error_handling PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME ErrorHandlingTests COMMAND test_error_handling)
set_tests_properties(ErrorHandlingTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_error_handling
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Plugin Interface Tests
add_executable(test_plugin_interface
    test_plugin_interface.cpp
)

target_include_directories(test_plugin_interface PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_plugin_interface PRIVATE
    Qt6::Test
    Qt6::Core
    QtPluginCore
)

add_test(NAME PluginInterfaceTests COMMAND test_plugin_interface)
set_tests_properties(PluginInterfaceTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS test_plugin_interface
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)
