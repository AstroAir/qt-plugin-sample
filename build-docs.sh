#!/bin/bash
# Build script for QtPlugin documentation
# Usage: ./build-docs.sh [serve|build|clean|deploy|check]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOCS_DIR="$SCRIPT_DIR/docs"
VENV_DIR="$SCRIPT_DIR/venv-docs"

# Default action
ACTION="${1:-serve}"

echo "QtPlugin Documentation Build Script"
echo "==================================="

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is not installed or not in PATH"
    echo "Please install Python 3.8 or later"
    exit 1
fi

# Create virtual environment if it doesn't exist
if [ ! -d "$VENV_DIR" ]; then
    echo "Creating Python virtual environment..."
    python3 -m venv "$VENV_DIR"
fi

# Activate virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Install/upgrade documentation dependencies
echo "Installing documentation dependencies..."
pip install --upgrade pip
pip install -r requirements-docs.txt

# Execute the requested action
case "$ACTION" in
    "serve")
        echo "Starting MkDocs development server..."
        echo "Open http://127.0.0.1:8000 in your browser"
        mkdocs serve --dev-addr=127.0.0.1:8000
        ;;
    "build")
        echo "Building documentation..."
        mkdocs build --clean --strict
        echo "Documentation built successfully in 'site' directory"
        ;;
    "clean")
        echo "Cleaning build artifacts..."
        rm -rf site/
        rm -rf .mkdocs_cache/
        echo "Clean completed"
        ;;
    "deploy")
        echo "Building and deploying documentation..."
        mkdocs gh-deploy --clean --strict
        echo "Documentation deployed successfully"
        ;;
    "check")
        echo "Checking documentation for issues..."
        mkdocs build --clean --strict --verbose
        echo "Documentation check passed"
        ;;
    *)
        echo "Unknown action: $ACTION"
        echo "Available actions:"
        echo "  serve  - Start development server (default)"
        echo "  build  - Build static documentation"
        echo "  clean  - Clean build artifacts"
        echo "  deploy - Deploy to GitHub Pages"
        echo "  check  - Check for documentation issues"
        exit 1
        ;;
esac

echo ""
echo "Done!"
