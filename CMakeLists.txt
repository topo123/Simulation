cmake_minimum_required(VERSION 3.29)
project(Sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

include_directories(./glad/include)

add_library(glad ./glad/src/glad.c)
add_library(arena ./src/arenas/Arena.cpp ./src/arenas/PoolArena.cpp)
add_library(material ./src/game/Material.cpp)
add_library(render ./src/game/Renderer.cpp)
add_library(updater ./src/game/Updater.cpp)

target_include_directories(arena PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(render PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(material PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(updater PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(utils ./src/shaders/shader_util.cpp )

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(utils PUBLIC material render updater arena)

find_package(glfw3 REQUIRED)

add_executable(main ./src/game/main.cpp)
target_link_libraries(main PRIVATE utils glfw glad)


