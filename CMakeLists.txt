cmake_minimum_required(VERSION 3.13)

project(networking)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED COMPONENTS Threads)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

add_library(websocket SHARED ${CMAKE_SOURCE_DIR}/src/WebSocketClient.cpp)
target_include_directories(websocket PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(websocket PUBLIC Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

add_subdirectory(tests)

install(TARGETS websocket DESTINATION lib)
install(FILES 
    ${CMAKE_SOURCE_DIR}/include/WebSocketClient.hpp 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/networking)
