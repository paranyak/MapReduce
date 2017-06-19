cmake_minimum_required(VERSION 3.6)
project(threads_project)

set(CMAKE_CXX_STANDARD 11)
#set(SOURCE_FILES map_reduce_words.cpp timing.cpp map_reduce_words.h)
set(SOURCE_FILES map_reduce_integral.cpp timing.cpp map_reduce_integral.h)


add_executable(threads_project ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(threads_project pthread)




