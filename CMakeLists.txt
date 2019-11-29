cmake_minimum_required(VERSION 3.12)
project(dupekill)

set(CMAKE_CXX_STANDARD 14)

include_directories("C:\\Users\\Burndi\\CLionProjects\\mrshv2")
link_directories("lib")
add_executable(dupekill main.cpp FileHash.cpp FileHash.h)
target_link_libraries(dupekill mrshv2)