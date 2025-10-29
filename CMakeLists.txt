cmake_minimum_required(VERSION 3.29)
project(Sim)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(RESOURCE_OUTPUT_DIR ${CMAKE_BINARY_DIR})
file(COPY ${RESOURCE_DIR} DESTINATION ${RESOURCE_OUTPUT_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_COMPILER g++)

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g -O0")
endif()

include_directories(./glad/include)
message(STATUS "${CMAKE_INCLUDE_DIRECTORIES}")

#set(GLFW_BUILD_SHARED_LIBS ON CACHE BOOL "Build GLFW as a shared library" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.4")

include_directories("/usr/include/x86_64-linux-gnu")

add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
add_library(arena ${CMAKE_CURRENT_SOURCE_DIR}/src/arenas/Arena.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/arenas/PoolArena.cpp)
add_library(world ${CMAKE_CURRENT_SOURCE_DIR}/src/game/World.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/game/ChunkHandler.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/game/Elements.cpp)
add_library(render ${CMAKE_CURRENT_SOURCE_DIR}/src/game/Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader_util.cpp)
add_library(input ${CMAKE_CURRENT_SOURCE_DIR}/src/game/InputHandler.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/arenas/PoolArena.cpp)


target_include_directories(arena PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(render PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(world PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(input PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(input PRIVATE glad)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")

add_executable(main ./src/game/main.cpp)

target_include_directories(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(main PRIVATE world render input arena glad glfw)


