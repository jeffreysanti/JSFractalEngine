cmake_minimum_required(VERSION 2.6)

project(JSFractalEngineServer C CXX)

FILE(GLOB_RECURSE src_c RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.c)
FILE(GLOB_RECURSE src_cxx RELATIVE ${CMAKE_HOME_DIRECTORY} src/*.cpp)
include_directories(src/CImg src/sqlite3 src/muParserC src/x264)

list(REMOVE_ITEM src_c src/sqlite3/sqlite3.c)

set(CMAKE_CXX_FLAGS "${EXFLAG}  -w -O3 -w -std=gnu++0x -D_GLIBCXX_USE_NANOSLEEP ")
set(CMAKE_C_FLAGS "${EXFLAG}  -w -O3 ")
set(CMAKE_EXE_LINKER_FLAGS "${EXFLAG} ")

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(PNG REQUIRED)



#link_directories(/usr/local/lib /usr/lib)
#target_link_libraries ( fractServ ${CMAKE_THREAD_LIBS_INIT} dl png )

add_executable(fractServ ${src_c} ${src_cxx})


TARGET_LINK_LIBRARIES(fractServ ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(fractServ ${PNG_LIBRARY})
target_link_libraries(fractServ ${CMAKE_DL_LIBS})

IF(${BUILD_TYP} MATCHES "lin64")
        target_link_libraries(fractServ ${CMAKE_HOME_DIRECTORY}/lib/sqlite3_64.a ${CMAKE_HOME_DIRECTORY}/lib/libjsoncpp_64.a )
ELSEIF(${BUILD_TYP} MATCHES "lin32")

ELSE()
        target_link_libraries(fractServ ${CMAKE_HOME_DIRECTORY}/lib/sqlite3_win32.lib  ${CMAKE_HOME_DIRECTORY}/lib/libjsoncpp_win32.a ws2_32 )
ENDIF(${BUILD_TYP} MATCHES "lin64")




