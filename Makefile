# Convenience commands for the native C build.
BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all configure build run test ci clean
all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel

run: build
	./$(BUILD_DIR)/pc/parasite-eve-pc

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

ci: test

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean
