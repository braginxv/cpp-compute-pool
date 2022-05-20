cmake_minimum_required(VERSION 3.17)
project(cpphack_common)

include_directories(src/cpp)

add_subdirectory(src/cpp)

enable_testing()
add_subdirectory(test/cpp)
