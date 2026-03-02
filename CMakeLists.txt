cmake_minimum_required(VERSION 3.16)
project(CAR-practica2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/*.cpp)
add_executable(CAR-practica2 ${SOURCES})

target_include_directories(CAR-practica2 PUBLIC include)

target_compile_options(CAR-practica2 PRIVATE -msse4.1)

# Sanitizers (correct list form)
set(SANITIZERS
    -fsanitize=address
    -fsanitize=undefined
    -fno-omit-frame-pointer
)

target_compile_options(CAR-practica2 PRIVATE ${SANITIZERS} -g)
target_link_options(CAR-practica2 PRIVATE ${SANITIZERS})
