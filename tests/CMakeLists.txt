cmake_minimum_required(VERSION 3.13)

find_package(GTest REQUIRED)

enable_testing()

add_executable(TestWebSocketClient EXCLUDE_FROM_ALL TestWebSocketClient.cpp)
target_include_directories(TestWebSocketClient PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TestWebSocketClient networking gtest)
add_test(NAME TestWebSocketClient COMMAND TestWebSocketClient)

add_executable(TestHTTPClient EXCLUDE_FROM_ALL TestHTTPClient.cpp)
target_include_directories(TestHTTPClient PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TestHTTPClient networking gtest)
add_test(NAME TestHTTPClient COMMAND TestHTTPClient)

add_custom_target(tests
    COMMAND make TestWebSocketClient
    COMMAND make TestHTTPClient)
