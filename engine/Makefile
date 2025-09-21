SRC_DIR = ./src
BUILD_DIR = ../build

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

INC_DIRS = $(shell find $(SRC_DIR) -type d)
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

CPPFLAGS = $(INC_FLAGS) -MMD -MP

PLATFORM ?= Linux
ifeq ($(PLATFORM), Linux)
	TARGET = libengine.so
	CC = gcc
	CFLAGS = -fpic -Wall -Werror -fvisibility=hidden
	LDFLAGS = -shared
	CPPFLAGS += -DLinux
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

$(BUILD_DIR)/$(TARGET): $(OBJS) $(BUILD_DIR)/engine.h
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/engine.h: $(SRC_DIR)/engine.h
	cp src/engine.h ../build

clean:
	rm -r $(BUILD_DIR)

.PHONY: clean noname

-include $(DEPS)