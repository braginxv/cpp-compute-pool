cmake_minimum_required(VERSION 3.17)

project(testing)

set(CMAKE_CXX_STANDARD 14)

#find_package(Boost COMPONENTS
#        context coroutine fiber thread container math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l REQUIRED)
find_package(GTest REQUIRED)
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads)

include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE TEST_SRC LIST_DIRECTORIES false "*.c" "*.cc" "*.cpp")

add_executable(cpphack_tests ${TEST_SRC})
target_link_libraries(cpphack_tests PUBLIC gtest gtest_main async_pool)

gtest_discover_tests(cpphack_tests)
