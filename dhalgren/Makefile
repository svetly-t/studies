# Writing the Makefile from scratch to avoid AI.
# I don't want any of that smoke.
# Heavily leaning on:
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html
# https://stackoverflow.com/questions/28285804/how-do-i-create-a-makefile-for-sdl2-c-on-windows

SRC_DIR := src
OBJ_DIR := obj

EXE := kid
SRC := $(wildcard $(SRC_DIR)/*.cc)
OBJ := $(SRC:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)

CXX := g++
CXXFLAGS := -std=c++11 -g -Iinclude -MMD -MP
BASELDFLAGS := -lSDL2main -lSDL2 -lSDL2_image

# If the builtin OS variable contains "Windows",
# then prepend lmingw32 to the libraries
ifeq ($(shell echo "$(OS)" | grep -oe Windows), Windows)
	LDFLAGS=-lmingw32 $(BASELDFLAGS)
else
	LDFLAGS=$(BASELDFLAGS)
endif

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $^ $(LDLIBS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

-include $(OBJ:.o=.d) # The dash silences errors when files don't exist (yet)

clean:
	rm -rf obj/* kid

# // Source - https://stackoverflow.com/q/28285804
# // Posted by PandaConda, modified by community. See post 'Timeline' for change history
# // Retrieved 2026-06-13, License - CC BY-SA 3.0

# CXX = g++
# CXXFLAGS = -std=c++0x -g -O3 -w -Wl,-subsystem,windows
# INCLFLAGS = -IC:\Libraries\i686-w64-mingw32\include\SDL2
# LDFLAGS = -LC:\Libraries\i686-w64-mingw32\lib -lmingw32 -lSDL2main -lSDL2
# OBJECTS = main.o
# TARGET = 1_hellosdl

# $(TARGET) : $(OBJECTS)
#     $(CXX) $(INCLFLAGS) $(LDFLAGS) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# main.o :

# clean:
#     rm -rf $(OBJECTS) $(TARGET)

# remake:
#     clean $(TARGET)
