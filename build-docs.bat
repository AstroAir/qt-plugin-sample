@echo off
REM Build script for QtPlugin documentation
REM Usage: build-docs.bat [serve|build|clean]

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set DOCS_DIR=%SCRIPT_DIR%docs
set VENV_DIR=%SCRIPT_DIR%venv-docs

REM Default action
set ACTION=%1
if "%ACTION%"=="" set ACTION=serve

echo QtPlugin Documentation Build Script
echo ===================================

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Error: Python is not installed or not in PATH
    echo Please install Python 3.8 or later
    exit /b 1
)

REM Create virtual environment if it doesn't exist
if not exist "%VENV_DIR%" (
    echo Creating Python virtual environment...
    python -m venv "%VENV_DIR%"
    if errorlevel 1 (
        echo Error: Failed to create virtual environment
        exit /b 1
    )
)

REM Activate virtual environment
echo Activating virtual environment...
call "%VENV_DIR%\Scripts\activate.bat"
if errorlevel 1 (
    echo Error: Failed to activate virtual environment
    exit /b 1
)

REM Install/upgrade documentation dependencies
echo Installing documentation dependencies...
pip install --upgrade pip
pip install -r requirements-docs.txt
if errorlevel 1 (
    echo Error: Failed to install dependencies
    exit /b 1
)

REM Execute the requested action
if "%ACTION%"=="serve" (
    echo Starting MkDocs development server...
    echo Open http://127.0.0.1:8000 in your browser
    mkdocs serve --dev-addr=127.0.0.1:8000
) else if "%ACTION%"=="build" (
    echo Building documentation...
    mkdocs build --clean --strict
    if errorlevel 1 (
        echo Error: Documentation build failed
        exit /b 1
    )
    echo Documentation built successfully in 'site' directory
) else if "%ACTION%"=="clean" (
    echo Cleaning build artifacts...
    if exist "site" rmdir /s /q "site"
    if exist ".mkdocs_cache" rmdir /s /q ".mkdocs_cache"
    echo Clean completed
) else if "%ACTION%"=="deploy" (
    echo Building and deploying documentation...
    mkdocs gh-deploy --clean --strict
    if errorlevel 1 (
        echo Error: Documentation deployment failed
        exit /b 1
    )
    echo Documentation deployed successfully
) else if "%ACTION%"=="check" (
    echo Checking documentation for issues...
    mkdocs build --clean --strict --verbose
    if errorlevel 1 (
        echo Error: Documentation has issues
        exit /b 1
    )
    echo Documentation check passed
) else (
    echo Unknown action: %ACTION%
    echo Available actions:
    echo   serve  - Start development server (default)
    echo   build  - Build static documentation
    echo   clean  - Clean build artifacts
    echo   deploy - Deploy to GitHub Pages
    echo   check  - Check for documentation issues
    exit /b 1
)

echo.
echo Done!
