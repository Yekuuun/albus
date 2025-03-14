cmake_minimum_required(VERSION 3.10)

project(albus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})