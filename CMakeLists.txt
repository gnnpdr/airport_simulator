cmake_minimum_required(VERSION 3.16)
project(sim CXX)

set(CMAKE_CXX_STANDARD 20)      #чтобы использовать contains для unordered_set
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

add_executable(sim main.cpp)