# Tests for Advanced Plugin Manager

# Find Qt6 Test component
find_package(Qt6 REQUIRED COMPONENTS Test)

# Enable automatic MOC
set(CMAKE_AUTOMOC ON)

# Test executable for basic functionality
add_executable(test_basic
    test_basic.cpp
)

target_link_libraries(test_basic
    Qt6::Test
    Qt6::Core
)

# Add test to CTest
add_test(NAME BasicTests COMMAND test_basic)

# Set test properties
set_tests_properties(BasicTests PROPERTIES
    TIMEOUT 30
)
