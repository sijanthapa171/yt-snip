# Makefile for yt-snip using CMake

# Compiler and flags
BUILD_DIR = build
PREFIX ?= /usr/local

.PHONY: all clean install uninstall

all: $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..
	@cd $(BUILD_DIR) && make

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)

install: all
	@cd $(BUILD_DIR) && make install

uninstall:
	@cd $(BUILD_DIR) && xargs rm -f < install_manifest.txt

# Development targets
format:
	@find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

lint:
	@find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy

.DEFAULT_GOAL := all 