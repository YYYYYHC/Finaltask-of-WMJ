cmake_minimum_required(VERSION 2.8)

project(identify)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${identify_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})




add_executable(identify src/identify.cpp)


link_directories({OpenCV_LIBRARY_DIRS})

target_link_libraries(identify ${OpenCV_LIBS} ${Boost_LIBS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

