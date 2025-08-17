# QtPlugin Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(QtPluginExamples VERSION 3.0.0)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core)

# Configuration Example
message(STATUS "Building QtPlugin examples")

# Add configuration example
add_subdirectory(configuration_example)

# Create plugins directory in build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/examples)

message(STATUS "Examples will be built in: ${CMAKE_BINARY_DIR}/plugins/examples")
