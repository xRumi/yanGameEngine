SRC_DIR = ./
BUILD_DIR = ./spv

SRCS = $(shell find $(SRC_DIR) -name '*.frag' -or -name '*.vert')
TARGETS = $(SRCS:%=$(BUILD_DIR)/%.spv)

all: $(TARGETS)

$(BUILD_DIR)/%.spv: %
	mkdir -p $(dir $@)
	glslc $< -o $@ -g

clean:
	rm -rf ./spv

# shaders:
# 	find . \( -name "*.frag" -o -name "*.vert" \) -exec glslc {} -o ./spv/{}.spv -g \;

