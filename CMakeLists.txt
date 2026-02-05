cmake_minimum_required(VERSION 3.16)
project(sim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(USE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
        
        add_compile_options(-fno-omit-frame-pointer -g)
    endif()
endif()

add_executable(sim 
    src/main.cpp
    src/field.cpp
    src/psg.cpp
    src/astar.cpp
)

target_include_directories(sim PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(sim PRIVATE -Wall -Wextra -Werror)