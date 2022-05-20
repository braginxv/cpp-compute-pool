cmake_minimum_required(VERSION 3.17)
project(async_pool)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS
        context coroutine fiber REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS ON)  # only find static libs
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

message(STATUS "Boost libraries version found: ${Boost_LIB_VERSION}")

file(GLOB_RECURSE PROJECT_SRC LIST_DIRECTORIES true "*.c" "*.cc" "*.cpp")

add_library(async_pool STATIC ${PROJECT_SRC})
target_link_libraries(async_pool PUBLIC ${Boost_LIBRARIES})