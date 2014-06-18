#!/bin/sh

/opt/centos/devtoolset-1.1/root/usr/bin/x86_64-redhat-linux-gcc -Wall -g -I./sqlite3 sqlite3/*.c -c
/opt/centos/devtoolset-1.1/root/usr/bin/x86_64-redhat-linux-g++ -Wall -g -I./muParserC -I./sqlite3 *.cpp muParserC/*.cpp sqlite3.o -o fractalServer -w -std=c++0x -ldl -lpthread -lpng -D_GLIBCXX_USE_NANOSLEEP


