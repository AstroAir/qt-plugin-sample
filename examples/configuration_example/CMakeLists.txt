# Configuration Example Plugin CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(ConfigurationExamplePlugin VERSION 1.0.0)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core)

# Enable automatic MOC
set(CMAKE_AUTOMOC ON)

# Create the plugin library
add_library(configuration_example_plugin SHARED
    configuration_example_plugin.hpp
    configuration_example_plugin.cpp
    configuration_example_plugin.json
)

# Set target properties
set_target_properties(configuration_example_plugin PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Include directories
target_include_directories(configuration_example_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

# Link libraries
target_link_libraries(configuration_example_plugin PRIVATE
    Qt6::Core
    QtPluginCore
)

# Install the plugin
install(TARGETS configuration_example_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtplugin/examples
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/qtplugin/examples
)

# Install plugin metadata
install(FILES configuration_example_plugin.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/qtplugin/examples
)

# Copy plugin to build directory for testing
add_custom_command(TARGET configuration_example_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:configuration_example_plugin>
    ${CMAKE_BINARY_DIR}/plugins/examples/
    COMMENT "Copying configuration example plugin to build directory"
)

# Copy metadata to build directory
add_custom_command(TARGET configuration_example_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration_example_plugin.json
    ${CMAKE_BINARY_DIR}/plugins/examples/
    COMMENT "Copying configuration example plugin metadata to build directory"
)

# Create the example application
add_executable(configuration_example_app
    main.cpp
)

# Set target properties for the application
set_target_properties(configuration_example_app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories for the application
target_include_directories(configuration_example_app PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

# Link libraries for the application
target_link_libraries(configuration_example_app PRIVATE
    Qt6::Core
    QtPluginCore
)

# Install the example application
install(TARGETS configuration_example_app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)
