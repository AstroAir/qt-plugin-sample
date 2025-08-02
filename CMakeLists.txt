cmake_minimum_required(VERSION 3.21)

# Project configuration
project(AdvancedPluginManager
    VERSION 2.1.0
    DESCRIPTION "Advanced Plugin Management System"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Quick
    QuickWidgets
    Network
    Concurrent
    Qml
    Gui
)

# Try to find Qt6 HttpServer and WebSockets (optional)
find_package(Qt6 COMPONENTS HttpServer WebSockets QUIET)
if(Qt6HttpServer_FOUND)
    message(STATUS "Qt6 HttpServer found - enabling HTTP server features")
    add_compile_definitions(QT_HTTPSERVER_AVAILABLE)
else()
    message(STATUS "Qt6 HttpServer not found - disabling HTTP server features")
endif()

if(Qt6WebSockets_FOUND)
    message(STATUS "Qt6 WebSockets found - enabling WebSocket features")
    add_compile_definitions(QT_WEBSOCKETS_AVAILABLE)
else()
    message(STATUS "Qt6 WebSockets not found - disabling WebSocket features")
endif()

# Try to find Qt6 Charts (optional)
find_package(Qt6 COMPONENTS Charts QUIET)
if(Qt6Charts_FOUND)
    message(STATUS "Qt6 Charts found - enabling chart features")
    add_compile_definitions(QT_CHARTS_AVAILABLE)
else()
    message(STATUS "Qt6 Charts not found - disabling chart features")
endif()

# Qt6 automatic handling
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Compiler-specific options
if(MSVC)
    # MSVC specific options
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    # Enable coroutines for MSVC
    add_compile_options(/await)
    
    # Set subsystem for Windows
    set(CMAKE_WIN32_EXECUTABLE ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC/Clang specific options
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Enable coroutines
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fcoroutines)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fcoroutines-ts)
    endif()
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(QT_QML_DEBUG)
    add_compile_definitions(DEBUG_BUILD)
endif()

# Release configuration optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
    add_compile_definitions(NDEBUG)
    
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3)
    endif()
endif()

# Platform-specific configurations
if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add subdirectories
add_subdirectory(src)

# Installation configuration
include(GNUInstallDirs)

# Install the executable
install(TARGETS AdvancedPluginManager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install Qt6 libraries (Windows)
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND ${WINDEPLOYQT_EXECUTABLE} --qmldir ${CMAKE_SOURCE_DIR}/qml \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/AdvancedPluginManager.exe
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL 0)
                message(WARNING \"windeployqt failed\")
            endif()
        ")
    endif()
endif()

# Create plugins directory
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins)

# Package configuration
set(CPACK_PACKAGE_NAME "AdvancedPluginManager")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Example Corporation")
set(CPACK_PACKAGE_CONTACT "support@example.com")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Advanced Plugin Manager")
    set(CPACK_NSIS_PACKAGE_NAME "AdvancedPluginManager")
    set(CPACK_NSIS_CONTACT "support@example.com")
    set(CPACK_NSIS_HELP_LINK "https://example.com/support")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://example.com")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Example Corporation")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev, qt6-declarative-dev")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Development")
endif()

include(CPack)

# Development tools and testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 COMPONENTS Test REQUIRED)
    add_subdirectory(tests)
endif()

# Documentation
option(BUILD_DOCUMENTATION "Build documentation" OFF)
if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Advanced Plugin Manager Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt6 version: ${Qt6_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build testing: ${BUILD_TESTING}")
message(STATUS "  Build documentation: ${BUILD_DOCUMENTATION}")
message(STATUS "")
