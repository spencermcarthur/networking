add_executable(BMHTTPClient BMHTTPClient.cpp)
target_link_libraries(BMHTTPClient PRIVATE networking benchmark)
target_compile_options(BMHTTPClient PRIVATE -O3)
