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
	./$(BUILD_DIR)/parasite-eve-pc --disc "$${PE_DISC_IMAGE:?Set PE_DISC_IMAGE to a USA CUE/BIN image}" $(RUN_ARGS)

test:
	$(CMAKE) -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON
	$(CMAKE) --build $(BUILD_DIR) --parallel
	ctest --test-dir $(BUILD_DIR) --output-on-failure --no-tests=error

ci: test

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean
