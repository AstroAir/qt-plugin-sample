@echo off
REM Build script for Advanced Plugin Manager on Windows

setlocal enabledelayedexpansion

echo Advanced Plugin Manager Build Script
echo ====================================

REM Check if CMake is available
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake and add it to your PATH
    exit /b 1
)

REM Check if Qt6 is available
where qmake >nul 2>&1
if errorlevel 1 (
    echo WARNING: Qt6 qmake not found in PATH
    echo Make sure Qt6 is installed and added to PATH
    echo You can also set Qt6_DIR environment variable
)

REM Parse command line arguments
set BUILD_TYPE=Release
set GENERATOR=Ninja
set CLEAN_BUILD=0

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="debug" set BUILD_TYPE=Debug
if /i "%~1"=="release" set BUILD_TYPE=Release
if /i "%~1"=="clean" set CLEAN_BUILD=1
if /i "%~1"=="vs2022" set GENERATOR=Visual Studio 17 2022
if /i "%~1"=="ninja" set GENERATOR=Ninja
shift
goto :parse_args
:done_parsing

echo Build Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Generator: %GENERATOR%
echo   Clean Build: %CLEAN_BUILD%
echo.

REM Set build directory
set BUILD_DIR=build\%BUILD_TYPE%

REM Clean build if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure
echo Configuring project...
cd "%BUILD_DIR%"
cmake -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..\..
if errorlevel 1 (
    echo ERROR: Configuration failed
    cd ..\..
    exit /b 1
)

REM Build
echo Building project...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: Build failed
    cd ..\..
    exit /b 1
)

cd ..\..

echo.
echo Build completed successfully!
echo Executable location: %BUILD_DIR%\src\AdvancedPluginManager.exe
echo.
echo To run the application:
echo   cd %BUILD_DIR%\src
echo   AdvancedPluginManager.exe
echo.

endlocal
