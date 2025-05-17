cmake_minimum_required(VERSION 3.29)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

add_executable(test test.cpp)

target_link_libraries(test PRIVATE world render arena  glad glfw Catch2::Catch2WithMain)
target_include_directories(test PRIVATE "../include")
