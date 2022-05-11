cmake_minimum_required(VERSION 3.17)
project(cpphack)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS
        context coroutine fiber thread container math_c99 math_c99f math_c99l math_tr1 math_tr1f math_tr1l REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS ON)  # only find static libs
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

if (Boost_FOUND)
    message(STATUS "Boost libraries have been found: ${Boost_LIB_VERSION}")
else ()
    message(FATAL_ERROR "No Boost libraries was found")
endif ()

file(GLOB PROJECT_SRC "*.c" "*.cc" "*.cpp")

add_executable(cpphack ${PROJECT_SRC})
target_link_libraries(cpphack PUBLIC ${Boost_LIBRARIES})
target_precompile_headers(cpphack PRIVATE <string> <sstream> <exception> <algorithm>)
#target_compile_definitions(cpphack PRIVATE
#        BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#        BOOST_THREAD_USES_MOVE
#        BOOST_THREAD_PROVIDES_EXECUTORS)
