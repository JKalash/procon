cmake_minimum_required(VERSION 3.7)
project(ProCon)

include_directories( ${MY_SOURCE_DIR} )

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -pthread")


add_executable(procon helper.cc main.cc)