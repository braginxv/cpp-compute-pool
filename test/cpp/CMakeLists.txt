cmake_minimum_required(VERSION 3.17)

project(testing)

set(CMAKE_CXX_STANDARD 14)

include_directories(${Boost_INCLUDE_DIR})

find_package(GTest REQUIRED)

find_package(Boost COMPONENTS
        context coroutine fiber REQUIRED)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads)

file(GLOB_RECURSE TEST_SRC LIST_DIRECTORIES false "*.c" "*.cc" "*.cpp")

add_executable(cpphack_tests ${TEST_SRC})
target_link_libraries(cpphack_tests PUBLIC gtest gtest_main async_pool)
include_directories(${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

gtest_discover_tests(cpphack_tests)
