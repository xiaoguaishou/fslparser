cmake_minimum_required(VERSION 3.17)
project(GenerateFiles)

set(CMAKE_CXX_STANDARD 14)

add_executable(GenerateFiles main.cpp)

add_executable(parse_file_meta parse_file_meta.c)