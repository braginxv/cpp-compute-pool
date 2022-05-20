cmake_minimum_required(VERSION 3.17)
project(cpphack)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(async)

file(GLOB_RECURSE PROJECT_SRC LIST_DIRECTORIES true "*.c" "*.cc" "*.cpp")

add_executable(cpphack ${PROJECT_SRC})
target_link_libraries(cpphack PUBLIC ${Boost_LIBRARIES} async_pool)
target_precompile_headers(cpphack PUBLIC <string> <sstream> <exception> <algorithm>)
#target_compile_definitions(cpphack PRIVATE
#        BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#        BOOST_THREAD_USES_MOVE
#        BOOST_THREAD_PROVIDES_EXECUTORS)
