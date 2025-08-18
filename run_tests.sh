#!/bin/bash

# QtPlugin Comprehensive Test Runner
# This script runs all tests for the QtPlugin system across different configurations

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
TEST_RESULTS_DIR="test_results"
COVERAGE_DIR="coverage"

# Test categories
RUN_BASIC_TESTS=true
RUN_COMPREHENSIVE_TESTS=true
RUN_CROSS_PLATFORM_TESTS=true
RUN_PERFORMANCE_TESTS=true
RUN_MEMORY_TESTS=true
RUN_INTEGRATION_TESTS=true
GENERATE_COVERAGE=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --basic-only)
            RUN_COMPREHENSIVE_TESTS=false
            RUN_CROSS_PLATFORM_TESTS=false
            RUN_PERFORMANCE_TESTS=false
            RUN_MEMORY_TESTS=false
            RUN_INTEGRATION_TESTS=false
            shift
            ;;
        --no-performance)
            RUN_PERFORMANCE_TESTS=false
            shift
            ;;
        --no-memory)
            RUN_MEMORY_TESTS=false
            shift
            ;;
        --coverage)
            GENERATE_COVERAGE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --help)
            echo "QtPlugin Test Runner"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --basic-only      Run only basic tests"
            echo "  --no-performance  Skip performance tests"
            echo "  --no-memory       Skip memory tests"
            echo "  --coverage        Generate code coverage report"
            echo "  --verbose         Verbose output"
            echo "  --build-dir DIR   Use custom build directory"
            echo "  --help            Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Helper functions
print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check prerequisites
check_prerequisites() {
    print_header "Checking Prerequisites"
    
    # Check if build directory exists
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory '$BUILD_DIR' not found. Please build the project first."
        exit 1
    fi
    
    # Check if CMake was configured with tests
    if [ ! -f "$BUILD_DIR/CTestTestfile.cmake" ]; then
        print_error "Tests not configured. Please configure CMake with -DQTPLUGIN_BUILD_TESTS=ON"
        exit 1
    fi
    
    # Check for required tools
    if ! command -v ctest &> /dev/null; then
        print_error "ctest not found. Please install CMake."
        exit 1
    fi
    
    if [ "$RUN_MEMORY_TESTS" = true ] && ! command -v valgrind &> /dev/null; then
        print_warning "valgrind not found. Memory tests will be skipped."
        RUN_MEMORY_TESTS=false
    fi
    
    if [ "$GENERATE_COVERAGE" = true ] && ! command -v gcov &> /dev/null; then
        print_warning "gcov not found. Coverage generation will be skipped."
        GENERATE_COVERAGE=false
    fi
    
    print_success "Prerequisites check completed"
}

# Create test results directory
setup_test_environment() {
    print_header "Setting Up Test Environment"
    
    mkdir -p "$TEST_RESULTS_DIR"
    mkdir -p "$COVERAGE_DIR"
    
    # Clean previous results
    rm -f "$TEST_RESULTS_DIR"/*.xml
    rm -f "$TEST_RESULTS_DIR"/*.log
    
    print_success "Test environment ready"
}

# Run basic tests
run_basic_tests() {
    if [ "$RUN_BASIC_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Basic Tests"
    
    cd "$BUILD_DIR"
    
    local test_args=""
    if [ "$VERBOSE" = true ]; then
        test_args="--verbose"
    fi
    
    # Run configuration manager tests
    if ctest -R "ConfigurationManagerTests" $test_args --output-on-failure; then
        print_success "Basic tests passed"
    else
        print_error "Basic tests failed"
        return 1
    fi
    
    cd ..
}

# Run comprehensive tests
run_comprehensive_tests() {
    if [ "$RUN_COMPREHENSIVE_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Comprehensive Tests"
    
    cd "$BUILD_DIR"
    
    local test_args=""
    if [ "$VERBOSE" = true ]; then
        test_args="--verbose"
    fi
    
    # Run comprehensive test suite
    if ctest -R "Comprehensive" $test_args --output-on-failure; then
        print_success "Comprehensive tests passed"
    else
        print_error "Comprehensive tests failed"
        return 1
    fi
    
    cd ..
}

# Run cross-platform tests
run_cross_platform_tests() {
    if [ "$RUN_CROSS_PLATFORM_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Cross-Platform Tests"
    
    cd "$BUILD_DIR"
    
    local test_args=""
    if [ "$VERBOSE" = true ]; then
        test_args="--verbose"
    fi
    
    if ctest -R "CrossPlatform" $test_args --output-on-failure; then
        print_success "Cross-platform tests passed"
    else
        print_error "Cross-platform tests failed"
        return 1
    fi
    
    cd ..
}

# Run performance tests
run_performance_tests() {
    if [ "$RUN_PERFORMANCE_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Performance Tests"
    
    cd "$BUILD_DIR"
    
    local test_args=""
    if [ "$VERBOSE" = true ]; then
        test_args="--verbose"
    fi
    
    if ctest -R "Performance" $test_args --output-on-failure; then
        print_success "Performance tests passed"
    else
        print_error "Performance tests failed"
        return 1
    fi
    
    cd ..
}

# Run memory tests
run_memory_tests() {
    if [ "$RUN_MEMORY_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Memory Tests"
    
    cd "$BUILD_DIR"
    
    local test_args=""
    if [ "$VERBOSE" = true ]; then
        test_args="--verbose"
    fi
    
    if ctest -R "Memory" $test_args --output-on-failure; then
        print_success "Memory tests passed"
    else
        print_error "Memory tests failed"
        return 1
    fi
    
    cd ..
}

# Run integration tests
run_integration_tests() {
    if [ "$RUN_INTEGRATION_TESTS" = false ]; then
        return 0
    fi
    
    print_header "Running Integration Tests"
    
    # Run service plugin integration tests
    if [ -f "$BUILD_DIR/examples/service-plugin/tests/ServicePluginIntegrationTests" ]; then
        cd "$BUILD_DIR/examples/service-plugin/tests"
        
        if ./ServicePluginIntegrationTests; then
            print_success "Service plugin integration tests passed"
        else
            print_error "Service plugin integration tests failed"
            return 1
        fi
        
        cd - > /dev/null
    else
        print_warning "Service plugin integration tests not found"
    fi
}

# Generate coverage report
generate_coverage() {
    if [ "$GENERATE_COVERAGE" = false ]; then
        return 0
    fi
    
    print_header "Generating Coverage Report"
    
    cd "$BUILD_DIR"
    
    # Generate coverage data
    if command -v gcov &> /dev/null; then
        find . -name "*.gcda" -exec gcov {} \;
        
        # Generate HTML report if lcov is available
        if command -v lcov &> /dev/null; then
            lcov --capture --directory . --output-file coverage.info
            lcov --remove coverage.info '/usr/*' --output-file coverage.info
            lcov --remove coverage.info '*/tests/*' --output-file coverage.info
            
            if command -v genhtml &> /dev/null; then
                genhtml coverage.info --output-directory "../$COVERAGE_DIR"
                print_success "Coverage report generated in $COVERAGE_DIR/"
            fi
        fi
    fi
    
    cd ..
}

# Generate test report
generate_test_report() {
    print_header "Generating Test Report"
    
    local report_file="$TEST_RESULTS_DIR/test_report.txt"
    
    {
        echo "QtPlugin Test Report"
        echo "===================="
        echo "Date: $(date)"
        echo "Platform: $(uname -a)"
        echo ""
        echo "Test Configuration:"
        echo "- Basic Tests: $RUN_BASIC_TESTS"
        echo "- Comprehensive Tests: $RUN_COMPREHENSIVE_TESTS"
        echo "- Cross-Platform Tests: $RUN_CROSS_PLATFORM_TESTS"
        echo "- Performance Tests: $RUN_PERFORMANCE_TESTS"
        echo "- Memory Tests: $RUN_MEMORY_TESTS"
        echo "- Integration Tests: $RUN_INTEGRATION_TESTS"
        echo ""
        
        if [ -f "$BUILD_DIR/Testing/Temporary/LastTest.log" ]; then
            echo "Test Results Summary:"
            echo "===================="
            tail -20 "$BUILD_DIR/Testing/Temporary/LastTest.log"
        fi
    } > "$report_file"
    
    print_success "Test report generated: $report_file"
}

# Main execution
main() {
    print_header "QtPlugin Comprehensive Test Suite"
    
    check_prerequisites
    setup_test_environment
    
    local exit_code=0
    
    # Run test suites
    run_basic_tests || exit_code=1
    run_comprehensive_tests || exit_code=1
    run_cross_platform_tests || exit_code=1
    run_performance_tests || exit_code=1
    run_memory_tests || exit_code=1
    run_integration_tests || exit_code=1
    
    # Generate reports
    generate_coverage
    generate_test_report
    
    # Final summary
    print_header "Test Summary"
    
    if [ $exit_code -eq 0 ]; then
        print_success "All tests passed successfully!"
    else
        print_error "Some tests failed. Check the logs for details."
    fi
    
    print_info "Test results available in: $TEST_RESULTS_DIR/"
    
    if [ "$GENERATE_COVERAGE" = true ] && [ -d "$COVERAGE_DIR" ]; then
        print_info "Coverage report available in: $COVERAGE_DIR/"
    fi
    
    exit $exit_code
}

# Run main function
main "$@"
