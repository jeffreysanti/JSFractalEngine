# General

INC_DIRS = -I./muParserC -I./sqlite3 -I./CImg
LIBS = -ldl -lpthread -lpng
LD = g++
LDFLAGS = -D_GLIBCXX_USE_NANOSLEEP

BUILD_DIR = ./build/
DIST_DIR = ./dist/

EXECUTABLE=fractserv

# Params For C Code

CC_C = gcc
CFLAGS_C = -c -Wall -g

# Params For C++ Code

CC_CPP = g++
CFLAGS_CPP = -c -Wall -g -D_GLIBCXX_USE_NANOSLEEP -w -std=c++0x

# FILES TO BUILD

SOURCES = Authentication.cpp ColorPalette.cpp DBSystem.cpp Fractal.cpp FractalGen.cpp FractalJulia.cpp FractalMultibrot.cpp ImageWriter.cpp \
			main.cpp Paramaters.cpp ParamaterSchema.cpp SocketClient.cpp muParserC/mucParserTokenReader.cpp \
			muParserC/mucParserError.cpp muParserC/mucParserDLL.cpp muParserC/mucParserCallback.cpp muParserC/mucParserBytecode.cpp muParserC/mucParserBase.cpp \
			muParserC/mucParser.cpp
			
SOURCES_C = sqlite3/sqlite3.c





OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

all: PREBUILD $(SOURCES) $(SOURCES_C) $(EXECUTABLE)

PREBUILD:
	mkdir $(BUILD_DIR)
	mkdir $(BUILD_DIR)/muParserC
	mkdir $(BUILD_DIR)/sqlite3
	mkdir $(DIST_DIR)
	
$(EXECUTABLE): $(OBJECTS) $(OBJECTS_C) 
	
	cd $(BUILD_DIR); $(LD) $(OBJECTS) $(OBJECTS_C) -o $@ $(LDFLAGS) $(LIBS)
	cp $(BUILD_DIR)/$@ $(DIST_DIR)
	cp ./etc/* $(DIST_DIR)

.cpp.o:
	$(CC_CPP) $(CFLAGS_CPP) $(INC_DIRS) $< -o $(BUILD_DIR)/$@

.c.o:
	$(CC_C) $(CFLAGS_C) $(INC_DIRS) $< -o $(BUILD_DIR)/$@
	

