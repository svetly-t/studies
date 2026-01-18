## Thanks GPT and this StackOverflow:
## https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

# =====================
# Compilers
# =====================
CXX := g++
EMXX := emcc

# =====================
# Common flags
# =====================
CXXFLAGS := -Iinclude -std=c++17 -g
LDFLAGS :=
LDLIBS := -lSDL2main -lSDL2 -lSDL2_image

# =====================
# Windows flags
# =====================
WIN_CXXFLAGS := -I/mingw64/include
WIN_LDFLAGS := -L/mingw64/lib
WIN_LDLIBS := -lmingw32 $(LDLIBS)

# =====================
# Emscripten flags
# =====================
EMCXXFLAGS := -Iinclude -std=c++17 \
              -sUSE_SDL=2 \
			  -sUSE_SDL_IMAGE=2 \
              -sALLOW_MEMORY_GROWTH=1 \
			  -sASSERTIONS \
			  -s SDL2_IMAGE_FORMATS='["png"]' \
			  -O3 --embed-file=./img/title.png@/img/title.png \
			  --shell-file emcc-shell.html
EMLDFLAGS :=
EMTARGET := kid.html

# =====================
# Files and directories
# =====================
SRC_DIR := src
OBJ_DIR := obj
EM_OBJ_DIR := obj-em
SRCS := main.cc $(wildcard $(SRC_DIR)/*.cc)
OBJS := $(SRCS:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)
EM_OBJS := $(SRCS:$(SRC_DIR)/%.cc=$(EM_OBJ_DIR)/%.o)
TARGET := kid

# =====================
# Targets
# =====================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Native objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------------
# Windows build
# ---------------------
win: CXXFLAGS += $(WIN_CXXFLAGS)
win: LDFLAGS  += $(WIN_LDFLAGS)
win: LDLIBS   := $(WIN_LDLIBS)
win: clean $(TARGET)

# ---------------------
# Emscripten build
# ---------------------
web: $(EMTARGET)

$(EMTARGET): $(EM_OBJS)
	$(EMXX) $(EMCXXFLAGS) $(EMLDFLAGS) -o $@ $^

$(EM_OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(EM_OBJ_DIR)
	$(EMXX) $(EMCXXFLAGS) -c $< -o $@

# =====================
# Cleanup
# =====================
clean:
	rm -rf $(OBJ_DIR) $(EM_OBJ_DIR) $(TARGET) $(EMTARGET)
