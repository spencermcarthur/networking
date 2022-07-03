cmake_minimum_required(VERSION 3.13)

project(networking_tests)

find_package(GTest REQUIRED)

enable_testing()

add_executable(test_websocketclient test_websocketclient.cpp)
target_include_directories(test_websocketclient PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_websocketclient websocket gtest)
add_test(NAME test_websocketclient COMMAND test_websocketclient)
