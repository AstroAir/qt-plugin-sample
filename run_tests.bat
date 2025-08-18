@echo off
setlocal enabledelayedexpansion

REM QtPlugin Comprehensive Test Runner for Windows
REM This script runs all tests for the QtPlugin system across different configurations

REM Configuration
set BUILD_DIR=build
set TEST_RESULTS_DIR=test_results
set COVERAGE_DIR=coverage

REM Test categories (default values)
set RUN_BASIC_TESTS=true
set RUN_COMPREHENSIVE_TESTS=true
set RUN_CROSS_PLATFORM_TESTS=true
set RUN_PERFORMANCE_TESTS=true
set RUN_MEMORY_TESTS=false
set RUN_INTEGRATION_TESTS=true
set GENERATE_COVERAGE=false
set VERBOSE=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if "%~1"=="--basic-only" (
    set RUN_COMPREHENSIVE_TESTS=false
    set RUN_CROSS_PLATFORM_TESTS=false
    set RUN_PERFORMANCE_TESTS=false
    set RUN_MEMORY_TESTS=false
    set RUN_INTEGRATION_TESTS=false
    shift
    goto :parse_args
)
if "%~1"=="--no-performance" (
    set RUN_PERFORMANCE_TESTS=false
    shift
    goto :parse_args
)
if "%~1"=="--no-memory" (
    set RUN_MEMORY_TESTS=false
    shift
    goto :parse_args
)
if "%~1"=="--coverage" (
    set GENERATE_COVERAGE=true
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set VERBOSE=true
    shift
    goto :parse_args
)
if "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    echo QtPlugin Test Runner for Windows
    echo.
    echo Usage: %0 [options]
    echo.
    echo Options:
    echo   --basic-only      Run only basic tests
    echo   --no-performance  Skip performance tests
    echo   --no-memory       Skip memory tests
    echo   --coverage        Generate code coverage report
    echo   --verbose         Verbose output
    echo   --build-dir DIR   Use custom build directory
    echo   --help            Show this help
    exit /b 0
)
echo Unknown option: %~1
exit /b 1

:args_done

REM Helper functions
:print_header
echo ================================
echo %~1
echo ================================
goto :eof

:print_success
echo [92m✓ %~1[0m
goto :eof

:print_warning
echo [93m⚠ %~1[0m
goto :eof

:print_error
echo [91m✗ %~1[0m
goto :eof

:print_info
echo [94mℹ %~1[0m
goto :eof

REM Check prerequisites
:check_prerequisites
call :print_header "Checking Prerequisites"

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    call :print_error "Build directory '%BUILD_DIR%' not found. Please build the project first."
    exit /b 1
)

REM Check if CMake was configured with tests
if not exist "%BUILD_DIR%\CTestTestfile.cmake" (
    call :print_error "Tests not configured. Please configure CMake with -DQTPLUGIN_BUILD_TESTS=ON"
    exit /b 1
)

REM Check for required tools
where ctest >nul 2>&1
if errorlevel 1 (
    call :print_error "ctest not found. Please install CMake."
    exit /b 1
)

call :print_success "Prerequisites check completed"
goto :eof

REM Setup test environment
:setup_test_environment
call :print_header "Setting Up Test Environment"

if not exist "%TEST_RESULTS_DIR%" mkdir "%TEST_RESULTS_DIR%"
if not exist "%COVERAGE_DIR%" mkdir "%COVERAGE_DIR%"

REM Clean previous results
del /q "%TEST_RESULTS_DIR%\*.xml" 2>nul
del /q "%TEST_RESULTS_DIR%\*.log" 2>nul

call :print_success "Test environment ready"
goto :eof

REM Run basic tests
:run_basic_tests
if "%RUN_BASIC_TESTS%"=="false" goto :eof

call :print_header "Running Basic Tests"

pushd "%BUILD_DIR%"

set test_args=
if "%VERBOSE%"=="true" set test_args=--verbose

REM Run configuration manager tests
ctest -R "ConfigurationManagerTests" %test_args% --output-on-failure
if errorlevel 1 (
    call :print_error "Basic tests failed"
    popd
    exit /b 1
)

call :print_success "Basic tests passed"
popd
goto :eof

REM Run comprehensive tests
:run_comprehensive_tests
if "%RUN_COMPREHENSIVE_TESTS%"=="false" goto :eof

call :print_header "Running Comprehensive Tests"

pushd "%BUILD_DIR%"

set test_args=
if "%VERBOSE%"=="true" set test_args=--verbose

REM Run comprehensive test suite
ctest -R "Comprehensive" %test_args% --output-on-failure
if errorlevel 1 (
    call :print_error "Comprehensive tests failed"
    popd
    exit /b 1
)

call :print_success "Comprehensive tests passed"
popd
goto :eof

REM Run cross-platform tests
:run_cross_platform_tests
if "%RUN_CROSS_PLATFORM_TESTS%"=="false" goto :eof

call :print_header "Running Cross-Platform Tests"

pushd "%BUILD_DIR%"

set test_args=
if "%VERBOSE%"=="true" set test_args=--verbose

ctest -R "CrossPlatform" %test_args% --output-on-failure
if errorlevel 1 (
    call :print_error "Cross-platform tests failed"
    popd
    exit /b 1
)

call :print_success "Cross-platform tests passed"
popd
goto :eof

REM Run performance tests
:run_performance_tests
if "%RUN_PERFORMANCE_TESTS%"=="false" goto :eof

call :print_header "Running Performance Tests"

pushd "%BUILD_DIR%"

set test_args=
if "%VERBOSE%"=="true" set test_args=--verbose

ctest -R "Performance" %test_args% --output-on-failure
if errorlevel 1 (
    call :print_error "Performance tests failed"
    popd
    exit /b 1
)

call :print_success "Performance tests passed"
popd
goto :eof

REM Run memory tests
:run_memory_tests
if "%RUN_MEMORY_TESTS%"=="false" goto :eof

call :print_header "Running Memory Tests"

pushd "%BUILD_DIR%"

set test_args=
if "%VERBOSE%"=="true" set test_args=--verbose

ctest -R "Memory" %test_args% --output-on-failure
if errorlevel 1 (
    call :print_error "Memory tests failed"
    popd
    exit /b 1
)

call :print_success "Memory tests passed"
popd
goto :eof

REM Run integration tests
:run_integration_tests
if "%RUN_INTEGRATION_TESTS%"=="false" goto :eof

call :print_header "Running Integration Tests"

REM Run service plugin integration tests
if exist "%BUILD_DIR%\examples\service-plugin\tests\ServicePluginIntegrationTests.exe" (
    pushd "%BUILD_DIR%\examples\service-plugin\tests"
    
    ServicePluginIntegrationTests.exe
    if errorlevel 1 (
        call :print_error "Service plugin integration tests failed"
        popd
        exit /b 1
    )
    
    call :print_success "Service plugin integration tests passed"
    popd
) else (
    call :print_warning "Service plugin integration tests not found"
)
goto :eof

REM Generate test report
:generate_test_report
call :print_header "Generating Test Report"

set report_file=%TEST_RESULTS_DIR%\test_report.txt

(
    echo QtPlugin Test Report
    echo ====================
    echo Date: %date% %time%
    echo Platform: %OS% %PROCESSOR_ARCHITECTURE%
    echo.
    echo Test Configuration:
    echo - Basic Tests: %RUN_BASIC_TESTS%
    echo - Comprehensive Tests: %RUN_COMPREHENSIVE_TESTS%
    echo - Cross-Platform Tests: %RUN_CROSS_PLATFORM_TESTS%
    echo - Performance Tests: %RUN_PERFORMANCE_TESTS%
    echo - Memory Tests: %RUN_MEMORY_TESTS%
    echo - Integration Tests: %RUN_INTEGRATION_TESTS%
    echo.
    
    if exist "%BUILD_DIR%\Testing\Temporary\LastTest.log" (
        echo Test Results Summary:
        echo ====================
        REM Show last 20 lines of test log
        powershell "Get-Content '%BUILD_DIR%\Testing\Temporary\LastTest.log' | Select-Object -Last 20"
    )
) > "%report_file%"

call :print_success "Test report generated: %report_file%"
goto :eof

REM Main execution
:main
call :print_header "QtPlugin Comprehensive Test Suite"

call :check_prerequisites
if errorlevel 1 exit /b 1

call :setup_test_environment

set exit_code=0

REM Run test suites
call :run_basic_tests
if errorlevel 1 set exit_code=1

call :run_comprehensive_tests
if errorlevel 1 set exit_code=1

call :run_cross_platform_tests
if errorlevel 1 set exit_code=1

call :run_performance_tests
if errorlevel 1 set exit_code=1

call :run_memory_tests
if errorlevel 1 set exit_code=1

call :run_integration_tests
if errorlevel 1 set exit_code=1

REM Generate reports
call :generate_test_report

REM Final summary
call :print_header "Test Summary"

if %exit_code% equ 0 (
    call :print_success "All tests passed successfully!"
) else (
    call :print_error "Some tests failed. Check the logs for details."
)

call :print_info "Test results available in: %TEST_RESULTS_DIR%\"

exit /b %exit_code%

REM Call main function
call :main %*
