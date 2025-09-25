# Examples CMakeLists.txt

# Simple trading example
add_executable(simple_trading simple_trading.c)
target_link_libraries(simple_trading ninja_trader_api)
target_include_directories(simple_trading PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Place order example
add_executable(place_order place_order.c)
target_link_libraries(place_order ninja_trader_api)
target_include_directories(place_order PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)