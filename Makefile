# Hyperliquid C SDK Makefile
# Production-ready build system

# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -O3 -fPIC
DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
INCLUDES = -Iinclude -I/opt/homebrew/include
LIBS = -lcurl -lcjson -lssl -lcrypto -lmsgpackc -lsecp256k1 -lm -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
TEST_DIR = tests
EXAMPLE_DIR = examples

# Library name
LIB_NAME = hyperliquid
STATIC_LIB = $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(LIB_NAME).so
SHARED_LIB_WIN = $(LIB_DIR)/lib$(LIB_NAME).dll
VERSION = 1.0.0

# Platform detection
ifeq ($(OS),Windows_NT)
    SHARED_LIB_TARGET = $(SHARED_LIB_WIN)
    LIB_EXT = dll
else
    SHARED_LIB_TARGET = $(SHARED_LIB)
    LIB_EXT = so
endif

# Source files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BIN_DIR)/%)

# Example files
EXAMPLE_SRCS = $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_BINS = $(EXAMPLE_SRCS:$(EXAMPLE_DIR)/%.c=$(BIN_DIR)/%)

# Default target
.PHONY: all
all: dirs $(STATIC_LIB) $(SHARED_LIB_TARGET)

# Create directories
.PHONY: dirs
dirs:
	@mkdir -p $(OBJ_DIR)/{crypto,msgpack,http}
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build static library
$(STATIC_LIB): $(OBJS)
	@echo "Building static library $(STATIC_LIB)"
	@$(AR) rcs $@ $^
	@echo "Static library built successfully"

# Build shared library
$(SHARED_LIB): $(OBJS)
	@echo "Building shared library $(SHARED_LIB)"
	@$(CC) -shared -o $@ $^ $(LIBS)
	@echo "Shared library built successfully"

# Build tests
.PHONY: tests
tests: $(STATIC_LIB)
	@echo "Building tests..."
	@for test in $(TEST_SRCS); do \
		test_name=$$(basename $$test .c); \
		echo "Building $$test_name"; \
		$(CC) $(CFLAGS) $(INCLUDES) $$test -o $(BIN_DIR)/$$test_name \
			-L$(LIB_DIR) -l$(LIB_NAME) $(LIBS); \
	done
	@echo "Tests built successfully"

# Run tests
.PHONY: test
test: tests
	@echo "Running tests..."
	@for test_bin in $(TEST_BINS); do \
		echo ""; \
		echo "=== Running $$test_bin ==="; \
		$$test_bin || exit 1; \
	done
	@echo ""
	@echo "All tests passed ✅"

# Build examples
.PHONY: examples
examples: $(STATIC_LIB)
	@echo "Building examples..."
	@for example in $(EXAMPLE_SRCS); do \
		example_name=$$(basename $$example .c); \
		echo "Building $$example_name"; \
		$(CC) $(CFLAGS) $(INCLUDES) $$example -o $(BIN_DIR)/$$example_name \
			-L$(LIB_DIR) -l$(LIB_NAME) $(LIBS); \
	done
	@echo "Examples built successfully"

# Debug build
.PHONY: debug
debug: CFLAGS = -std=c11 -Wall -Wextra -pedantic $(DEBUG_FLAGS) -fPIC
debug: clean all

# Install library
.PHONY: install
install: all
	@echo "Installing library to /usr/local..."
	@sudo cp $(STATIC_LIB) /usr/local/lib/
	@sudo cp $(SHARED_LIB) /usr/local/lib/
	@sudo cp include/*.h /usr/local/include/
	@sudo ldconfig 2>/dev/null || true
	@echo "Library installed successfully"

# Uninstall library
.PHONY: uninstall
uninstall:
	@echo "Uninstalling library..."
	@sudo rm -f /usr/local/lib/lib$(LIB_NAME).*
	@sudo rm -f /usr/local/include/hyperliquid.h
	@sudo rm -f /usr/local/include/hl_*.h
	@echo "Library uninstalled"

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Format code
.PHONY: format
format:
	@echo "Formatting code..."
	@find $(SRC_DIR) -name '*.c' -o -name '*.h' | xargs clang-format -i
	@find include -name '*.h' | xargs clang-format -i
	@echo "Formatting complete"

# Run static analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@cppcheck --enable=all --suppress=missingIncludeSystem \
		-I include $(SRC_DIR) $(TEST_DIR)
	@echo "Analysis complete"

# Generate documentation
.PHONY: docs
docs:
	@echo "Generating documentation..."
	@doxygen Doxyfile 2>/dev/null || echo "Doxygen not found, skipping"
	@echo "Documentation generated in docs/"

# Memory check with valgrind
.PHONY: memcheck
memcheck: tests
	@echo "Running memory checks..."
	@for test_bin in $(TEST_BINS); do \
		echo "Checking $$test_bin"; \
		valgrind --leak-check=full --show-leak-kinds=all \
			--track-origins=yes $$test_bin; \
	done

# Coverage report
.PHONY: coverage
coverage: CFLAGS += --coverage
coverage: clean test
	@echo "Generating coverage report..."
	@gcov $(SRCS)
	@lcov --capture --directory . --output-file coverage.info
	@genhtml coverage.info --output-directory coverage
	@echo "Coverage report generated in coverage/"

# Help
.PHONY: help
help:
	@echo "Hyperliquid C SDK - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build static and shared libraries (default)"
	@echo "  tests      - Build test executables"
	@echo "  test       - Run all tests"
	@echo "  examples   - Build example programs"
	@echo "  debug      - Build with debug symbols and sanitizers"
	@echo "  install    - Install library to /usr/local"
	@echo "  uninstall  - Remove library from /usr/local"
	@echo "  clean      - Remove build artifacts"
	@echo "  format     - Format code with clang-format"
	@echo "  analyze    - Run static analysis with cppcheck"
	@echo "  docs       - Generate documentation with doxygen"
	@echo "  memcheck   - Run tests with valgrind"
	@echo "  coverage   - Generate code coverage report"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build library"
	@echo "  make test              # Build and run tests"
	@echo "  make debug test        # Debug build with tests"
	@echo "  make install           # Install library"

.PHONY: info
info:
	@echo "Build configuration:"
	@echo "  CC:       $(CC)"
	@echo "  CFLAGS:   $(CFLAGS)"
	@echo "  INCLUDES: $(INCLUDES)"
	@echo "  LIBS:     $(LIBS)"
	@echo "  VERSION:  $(VERSION)"
	@echo "  SOURCES:  $(words $(SRCS)) files"
	@echo "  TESTS:    $(words $(TEST_SRCS)) files"
	@echo "  EXAMPLES: $(words $(EXAMPLE_SRCS)) files"

