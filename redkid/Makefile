## Thanks GPT and this StackOverflow:
## https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

# Compiler and flags
CXX := g++ -g
CXXFLAGS := -Iinclude -std=c++17
LDFLAGS := -L/usr/local/lib
LDLIBS := -lSDL2main -lSDL2 -lSDL2_image
WIN_LDLIBS := -lmingw32 $(LDLIBS)

# Files and directories
SRC_DIR := src
OBJ_DIR := obj
INCLUDE_DIR := include
SRCS := game.cc $(wildcard $(SRC_DIR)/*.cc)
OBJS := $(SRCS:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)
TARGET := kid

# Default target
all: $(TARGET)

# Create the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Windows-specific build
win: CXXFLAGS += -I/mingw64/include
win: LDFLAGS += -L/mingw64/lib
win: LDLIBS = $(WIN_LDLIBS)
win: $(TARGET)
