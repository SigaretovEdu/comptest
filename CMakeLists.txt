cmake_minimum_required(VERSION 3.10)

project(comptest)

add_compile_options(-Wall -Wextra -Wpedantic -g)

set(SOURCE comptest.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})
