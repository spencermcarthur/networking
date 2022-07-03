cmake_minimum_required(VERSION 3.13)

project(networking)

find_package(Threads REQUIRED COMPONENTS Threads)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

add_library(websocket SHARED ${CMAKE_SOURCE_DIR}/src/websocketclient.cpp)
target_include_directories(websocket PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(websocket PUBLIC Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

add_subdirectory(tests)
