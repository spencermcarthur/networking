cmake_minimum_required(VERSION 3.13)

project(networking)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra)
message(STATUS "Configuration: ${CMAKE_BUILD_TYPE}")

if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    add_compile_options(-ggdb)
    add_compile_definitions(DEBUG=TRUE)
else()
    add_compile_options(-O3)
endif()

find_package(Threads REQUIRED COMPONENTS Threads)
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

add_library(websocket SHARED ${CMAKE_SOURCE_DIR}/src/WebSocketClient.cpp)
target_include_directories(websocket PUBLIC ${CMAKE_SOURCE_DIR}/include/networking)
target_link_libraries(websocket PUBLIC Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

add_library(http SHARED ${CMAKE_SOURCE_DIR}/src/HTTPClient.cpp)
target_include_directories(http PUBLIC ${CMAKE_SOURCE_DIR}/include/networking)
target_link_libraries(http PUBLIC Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

add_library(networking SHARED ${CMAKE_SOURCE_DIR}/src/HTTPClient.cpp ${CMAKE_SOURCE_DIR}/src/WebSocketClient.cpp)
target_include_directories(networking PUBLIC ${CMAKE_SOURCE_DIR}/include/networking)
target_link_libraries(networking PUBLIC Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

add_subdirectory(tests)

install(TARGETS websocket http networking DESTINATION lib)
install(FILES 
    ${CMAKE_SOURCE_DIR}/include/networking/WebSocketClient.hpp 
    ${CMAKE_SOURCE_DIR}/include/networking/HTTPClient.hpp 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/networking)
