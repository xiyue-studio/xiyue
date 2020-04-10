cmake_minimum_required(VERSION 2.8)
project(xiyue)


set (TBD "tmp")

SET (CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wall -g -fshort-wchar -fwide-exec-charset=UTF-16LE -m64")
SET (CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -Wall -O2 -fshort-wchar -fwide-exec-charset=UTF-16LE -m64")

include_directories("include" "src")
file (GLOB SRC "src/*.cpp")

add_library(xiyue ${SRC})
