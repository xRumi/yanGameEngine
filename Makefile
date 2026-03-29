SRC_DIR = ./src
BUILD_DIR = ./build
LIB_DIR = ./lib

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

INC_DIRS = $(shell find $(SRC_DIR) -type d) $(LIB_DIR)
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

CPPFLAGS = $(INC_FLAGS) -MMD -MP

PLATFORM ?= Linux
ifeq ($(PLATFORM), Linux)
	TARGET = main
	CC = gcc
	CFLAGS = -fpic -Wall
	LDFLAGS = -lvulkan -lxkbcommon -lm
	PLATFORMFLAGS += -lwayland-client
else
noname:
	@echo "Platform '$(PLATFORM)' not supported, exiting"
	@exit
endif

BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g
else
	CPPFLAGS += -DNDEBUG
endif

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(PLATFORMFLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

shaders:
	$(MAKE) -C ./assets/shaders

run: $(BUILD_DIR)/$(TARGET) shaders
	$(BUILD_DIR)/main

clean:
	rm -r $(BUILD_DIR)

.PHONY: clean noname

-include $(DEPS)