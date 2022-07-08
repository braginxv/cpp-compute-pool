cmake_minimum_required(VERSION 3.17)
project(cpphack)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(async)

find_package(Boost COMPONENTS
        context coroutine fiber REQUIRED)

file(GLOB_RECURSE PROJECT_SRC LIST_DIRECTORIES true "*.c" "*.cc" "*.cpp")

add_executable(cpphack ${PROJECT_SRC})
target_link_libraries(cpphack PUBLIC ${Boost_LIBRARIES} async_pool)
target_precompile_headers(cpphack PUBLIC <string> <sstream> <exception> <algorithm>)
include_directories(${Boost_INCLUDE_DIR})
