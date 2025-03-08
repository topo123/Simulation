cmake_minimum_required(VERSION 3.29)
project(Sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory  -fno-omit-frame-pointer -g")
#set(ENV{ASAN_OPTIONS=suppressions} "${CMAKE_CURRENT_SOURCE_DIR}/asan.supp")

include_directories(./glad/include)
message(STATUS "${CMAKE_INCLUDE_DIRECTORIES}")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4")

include_directories("/usr/include/x86_64-linux-gnu")

add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
add_library(arena ${CMAKE_CURRENT_SOURCE_DIR}/src/arenas/Arena.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/arenas/PoolArena.cpp)
add_library(world ${CMAKE_CURRENT_SOURCE_DIR}/src/game/World.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/game/ChunkHandler.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/game/Elements.cpp)
add_library(render ${CMAKE_CURRENT_SOURCE_DIR}/src/game/Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader_util.cpp)
add_library(utilities ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/SwapbackArray.cpp)


target_include_directories(arena PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(render PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(world PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(utilities PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(main ./src/game/main.cpp)

target_include_directories(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(main PRIVATE world render arena utilities glad glfw)


