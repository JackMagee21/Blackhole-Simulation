# Compiler & Flags
CXX          := g++
CXXFLAGS     := -std=c++17 -Wall -Wextra -pedantic -MMD -MP -fopenmp
OPTFLAGS     := -O3 -march=native -ffast-math
DEBUGFLAGS   := -g -DDEBUG

# Libraries and external flags
SDL_CFLAGS   := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS     := $(shell sdl2-config --libs 2>/dev/null)

# Fallback in case sdl2-config isn't present
ifeq ($(strip $(SDL_LIBS)),)
    SDL_LIBS := -lSDL2
endif

INCLUDES := -I. -Iinclude -Isrc $(SDL_CFLAGS)
LIBS         := $(SDL_LIBS) -lm -fopenmp

# Output binary name
TARGET       := sim

# Directories
BUILD_DIR    := build
SRC_DIRS     := src Window


# Find all .cpp files in the source directories (and their subdirectories)
SRCS         := $(shell find $(SRC_DIRS) -name "*.cpp" 2>/dev/null)
OBJS         := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS         := $(OBJS:.o=.d)

all: CXXFLAGS += $(OPTFLAGS)
all: $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking binary: $@"
	@$(CXX) $(OBJS) -o $@ $(LIBS)
	@echo "Build complete -> ./$@"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: all
	@./$(TARGET)

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(TARGET) render.ppm
	@echo "Clean complete."

# Include auto-generated header dependency rules (.d files)
-include $(DEPS)

.PHONY: all debug run clean