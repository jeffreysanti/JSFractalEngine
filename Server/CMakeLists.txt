cmake_minimum_required(VERSION 3.0)

project(JSFractalEngineServer C CXX)

FILE(GLOB_RECURSE src_c RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.c)
FILE(GLOB_RECURSE src_cxx RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.cpp)
include_directories(src/CImg src/sqlite3 src/muParserC)

set(CMAKE_CXX_FLAGS "${EXFLAG}  -w -O3 -w -std=gnu++0x -D_GLIBCXX_USE_NANOSLEEP ")
set(CMAKE_C_FLAGS "${EXFLAG}  -w -O3 ")
set(CMAKE_EXE_LINKER_FLAGS "${EXFLAG} -ldl -lpthread -lpng ")

#FIND_PACKAGE ( Threads REQUIRED )
#FIND_PACKAGE ( Dl REQUIRED )
#FIND_PACKAGE ( PNG REQUIRED )

#link_directories(/usr/local/lib /usr/lib)
#target_link_libraries ( fractServ ${CMAKE_THREAD_LIBS_INIT} dl png )

add_executable(fractServ ${src_c} ${src_cxx})

