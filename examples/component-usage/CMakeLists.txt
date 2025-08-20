cmake_minimum_required(VERSION 3.16)
project(QtPluginComponentUsageExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Find QtPlugin library
find_package(PkgConfig REQUIRED)
pkg_check_modules(QTPLUGIN REQUIRED qtplugin)

# Create executable
add_executable(component_usage_example
    main.cpp
)

# Link libraries
target_link_libraries(component_usage_example
    Qt6::Core
    ${QTPLUGIN_LIBRARIES}
)

# Include directories
target_include_directories(component_usage_example PRIVATE
    ${QTPLUGIN_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(component_usage_example PRIVATE
    ${QTPLUGIN_CFLAGS_OTHER}
)

# Set output directory
set_target_properties(component_usage_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS component_usage_example
    RUNTIME DESTINATION bin
)

# Copy example to build directory for easy testing
add_custom_command(TARGET component_usage_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_BINARY_DIR}/examples/component-usage/
)
