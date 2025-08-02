#!/bin/bash

# Build script for Advanced Plugin Manager on Unix-like systems

set -e

echo "Advanced Plugin Manager Build Script"
echo "===================================="

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed or not in PATH"
    echo "Please install CMake"
    exit 1
fi

# Check if Qt6 is available
if ! command -v qmake &> /dev/null; then
    echo "WARNING: Qt6 qmake not found in PATH"
    echo "Make sure Qt6 is installed and added to PATH"
    echo "You can also set Qt6_DIR environment variable"
fi

# Default values
BUILD_TYPE="Release"
GENERATOR="Ninja"
CLEAN_BUILD=0
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        release)
            BUILD_TYPE="Release"
            shift
            ;;
        clean)
            CLEAN_BUILD=1
            shift
            ;;
        ninja)
            GENERATOR="Ninja"
            shift
            ;;
        make)
            GENERATOR="Unix Makefiles"
            shift
            ;;
        xcode)
            GENERATOR="Xcode"
            shift
            ;;
        -j)
            JOBS="$2"
            shift 2
            ;;
        --jobs=*)
            JOBS="${1#*=}"
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  debug       Build in Debug mode"
            echo "  release     Build in Release mode (default)"
            echo "  clean       Clean build directory before building"
            echo "  ninja       Use Ninja generator (default)"
            echo "  make        Use Unix Makefiles generator"
            echo "  xcode       Use Xcode generator (macOS only)"
            echo "  -j N        Use N parallel jobs"
            echo "  --jobs=N    Use N parallel jobs"
            echo "  -h, --help  Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

echo "Build Configuration:"
echo "  Build Type: $BUILD_TYPE"
echo "  Generator: $GENERATOR"
echo "  Clean Build: $CLEAN_BUILD"
echo "  Parallel Jobs: $JOBS"
echo

# Set build directory
BUILD_DIR="build/$BUILD_TYPE"

# Clean build if requested
if [[ $CLEAN_BUILD -eq 1 ]]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure
echo "Configuring project..."
cd "$BUILD_DIR"
cmake -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ../..

# Build
echo "Building project..."
if [[ "$GENERATOR" == "Ninja" ]]; then
    cmake --build . --parallel "$JOBS"
else
    cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS"
fi

cd ../..

echo
echo "Build completed successfully!"
echo "Executable location: $BUILD_DIR/src/AdvancedPluginManager"
echo

# Make executable runnable on macOS
if [[ "$OSTYPE" == "darwin"* ]] && [[ "$GENERATOR" == "Xcode" ]]; then
    echo "To run the application:"
    echo "  open $BUILD_DIR/src/$BUILD_TYPE/AdvancedPluginManager.app"
else
    echo "To run the application:"
    echo "  cd $BUILD_DIR/src"
    echo "  ./AdvancedPluginManager"
fi
echo
