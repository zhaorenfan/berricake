cmake_minimum_required(VERSION 3.31)
project(berricake)
set(CMAKE_CXX_STANDARD 20)

include(cmakeconf/compiler_conf.cmake)
include(cmakeconf/building_output.cmake)

add_subdirectory(src)