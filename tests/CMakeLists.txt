cmake_minimum_required(VERSION 3.13)

project(networking_tests)

find_package(GTest REQUIRED)

enable_testing()

add_executable(TestWebSocketClient TestWebSocketClient.cpp)
target_include_directories(TestWebSocketClient PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(TestWebSocketClient websocket gtest)
add_test(NAME TestWebSocketClient COMMAND TestWebSocketClient)

add_executable(TestHTTPClient TestHTTPClient.cpp)
target_include_directories(TestHTTPClient PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(TestHTTPClient http gtest)
add_test(NAME TestHTTPClient COMMAND TestHTTPClient)
