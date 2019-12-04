cmake_minimum_required(VERSION 3.12)
project(dupekill)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/mrshv2)


add_executable(dupekill main.cpp src/FileHash.cpp include/FileHash.h)
target_link_libraries(dupekill PRIVATE mrshv2 stdc++fs)
