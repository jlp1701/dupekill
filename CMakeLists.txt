cmake_minimum_required(VERSION 3.12)
project(dupekill)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/mrshv2)
add_subdirectory(lib/spdlog)

add_executable(dupekill main.cpp src/FileHash.cpp include/FileHash.h include/def.h)
target_link_libraries(dupekill mrshv2 stdc++fs spdlog)
