cmake_minimum_required(VERSION 3.16)
project(CAR-practica2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB SOURCES src/*.cpp)

add_executable(CAR-practica2 ${SOURCES})

# Include headers
target_include_directories(CAR-practica2 PUBLIC include)