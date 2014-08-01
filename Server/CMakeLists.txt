cmake_minimum_required(VERSION 2.0)

project(JSFractalEngineServer)

FILE(GLOB_RECURSE src_c RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.c)
FILE(GLOB_RECURSE src_cxx RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.cpp)

set(CMAKE_CXX_FLAGS ${EXFLAG} " -w -O3 -w -std=c++0x")
set(CMAKE_C_FLAGS ${EXFLAG} " -w -O3")
set(CMAKE_EXE_LINKER_FLAGS ${EXFLAG} " -ldl -lpthread -lpng -D_GLIBCXX_USE_NANOSLEEP")

add_executable(fractServ ${src_c} ${src_cxx})

