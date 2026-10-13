# Convenience commands for the native C build.
BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all configure build run ci clean
all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

run: build
	./$(BUILD_DIR)/pc/parasite-eve-pc

ci: build

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean
