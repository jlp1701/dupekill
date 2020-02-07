cmake_minimum_required(VERSION 3.12)
project(dupekill)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.65.0 COMPONENTS program_options)

if(Boost_FOUND)
    add_subdirectory(lib/mrshv2)
    add_subdirectory(lib/spdlog)
    add_executable(dupekill main.cpp src/FileHash.cpp include/FileHash.h include/def.h)
    target_link_libraries(dupekill mrshv2 stdc++fs spdlog ${Boost_LIBRARIES})
endif()