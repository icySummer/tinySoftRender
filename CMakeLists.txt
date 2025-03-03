#要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
#项目名称
project(tinySoftRender  LANGUAGES CXX)
#C++标准
set(CMAKE_CXX_STANDARD 17)

set(HEADERS
 "src/model.h"
 "src/tgaimage.h"
 "src/geometry.h"
 "src/our_gl.h"
)

set(SOURCES
 "src/main.cpp"
 "src/model.cpp"
 "src/geometry.cpp"
 "src/tgaimage.cpp"
 "src/our_gl.cpp"
 )
 set(TARGET tinySoftRender)
add_executable(${TARGET} ${HEADERS} ${SOURCES})