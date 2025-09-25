# Tests CMakeLists.txt

# Basic tests
add_executable(test_basic test_basic.c)
target_link_libraries(test_basic ninja_trader_api)
target_include_directories(test_basic PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Add test to CTest
add_test(NAME basic_tests COMMAND test_basic)