cmake_minimum_required(VERSION 3.10)
project(ninja_trader_api VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    message(FATAL_ERROR "libcurl not found. Please install libcurl development package.")
endif()

# Add cJSON subdirectory
add_subdirectory(cJSON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cJSON)

# Source files
set(NINJA_SOURCES
    src/ninja_api.c
    src/ninja_auth.c
    src/ninja_client.h
    src/ninja_platform.h
    src/ninja_orders.c
    src/ninja_account.c
    src/ninja_positions.c
    src/ninja_contracts.c
)

# Create library
add_library(ninja_trader_api STATIC ${NINJA_SOURCES})

# Set target properties
set_target_properties(ninja_trader_api PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/ninja/ninja_api.h;include/ninja/ninja_types.h"
)

# Link libraries
target_link_libraries(ninja_trader_api
    CURL::libcurl
    cjson
)

# Compiler flags
target_compile_options(ninja_trader_api PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(ninja_trader_api PRIVATE
        _WIN32_WINNT=0x0601
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(ninja_trader_api ws2_32)

    # Windows examples need the same definitions
    if(BUILD_EXAMPLES)
        target_compile_definitions(simple_trading PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(place_order PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    # Windows tests need the same definitions
    if(BUILD_TESTS)
        target_compile_definitions(test_basic PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()

# Install targets
install(TARGETS ninja_trader_api
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/ninja
)

# Install additional headers
install(DIRECTORY include/ninja/
    DESTINATION include/ninja
    FILES_MATCHING PATTERN "*.h"
)

# Create pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ninja_trader_api.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ninja_trader_api.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ninja_trader_api.pc
    DESTINATION lib/pkgconfig
)

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests (optional)
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print summary
message(STATUS "")
message(STATUS "NinjaTrader API Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "")