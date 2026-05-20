SRC_DIR = ./src
BUILD_DIR = ./build
ExINC_DIR = ./includes
ExLIB_DIR = ./libs

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

INC_DIRS = $(shell find $(SRC_DIR) -type d) $(ExINC_DIR)
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

CPPFLAGS = $(INC_FLAGS) -MMD -MP
LDFLAGS  = -L$(ExLIB_DIR)

USE_GLFW = aye

ifeq ($(OS), Windows_NT)
	PLATFORM ?= Windows
else
	PLATFORM ?= $(shell uname -s)
endif

ifeq ($(PLATFORM), Linux)
	TARGET = main
	CC = gcc
	CFLAGS += -fpic -Wall
	LDFLAGS += -lvulkan -lxkbcommon -lm -pthread
	ifdef USE_GLFW
		CPPFLAGS += -DUSE_GLFW
		LDFLAGS += -lglfw
	else ifdef WAYLAND_DISPLAY
		CPPFLAGS += -DUSE_WAYLAND
		LDFLAGS += -lwayland-client
	endif
else ifeq ($(PLATFORM), Windows)
	TARGET = main.exe
	CC = gcc
	CFLAGS += -fpic -Wall
	LDFLAGS += -lvulkan-1 -lglfw3 -lgdi32 -lm -pthread
	CPPFLAGS += -DUSE_GLFW
endif

BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g
else
	CPPFLAGS += -DNDEBUG
endif

$(BUILD_DIR)/$(TARGET): $(OBJS) shaders
	$(CC) $(OBJS) $(LDFLAGS) $(DEFINES) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

shaders:
	$(MAKE) -C ./assets/shaders

run: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/main

clean:
	rm -r $(BUILD_DIR)

.PHONY: clean

-include $(DEPS)