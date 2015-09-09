cmake_minimum_required(VERSION 3.0)
project(physica)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
set(SOURCE_FILES main.cpp)
add_executable(physica ${SOURCE_FILES})
target_link_libraries(physica "-lrt -lm -ldl -lxcb -lxcb-image -lxcb-icccm -lxcb-keysyms -lasound")
