cmake_minimum_required(VERSION 3.16)
project(sim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sim 
    src/main.cpp
    src/field.cpp
    src/psg.cpp
)

target_include_directories(sim PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)