SRC_DIR = ./src
BUILD_DIR = ./build

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

INC_DIRS = $(shell find $(SRC_DIR) -type d)
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

CPPFLAGS = $(INC_FLAGS) -MMD -MP

PLATFORM ?= Linux
ifeq ($(PLATFORM), Linux)
	TARGET = test
	CC = gcc
	CFLAGS = -Wall -Werror -I../build
	LDFLAGS = -L../build -lengine -Wl,-rpath=../build
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

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

.PHONY: clean

-include $(DEPS)