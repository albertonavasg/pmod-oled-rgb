# Toolchain
TOOLCHAIN_PATH := $(HOME)/tools/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin

CC  := $(TOOLCHAIN_PATH)/arm-none-linux-gnueabihf-gcc
CXX := $(TOOLCHAIN_PATH)/arm-none-linux-gnueabihf-g++

BUILD_DIR := build

.PHONY: all compile send clean

# Default target
all: compile send

# Compile Screen Test programs
compile:
	@mkdir -p $(BUILD_DIR)

	@echo "Compiling Screen Test program in C..."
	@$(CC) mem.c -o $(BUILD_DIR)/mem_c
	@$(CC) uio.c -o $(BUILD_DIR)/uio_c

	@echo "Compiling Screen Test program in C++..."
	@$(CXX) mem.cpp -o $(BUILD_DIR)/mem_cpp
	@$(CXX) uio.cpp -o $(BUILD_DIR)/uio_cpp

	@echo "Compilation finished"
	@echo "Generated binaries:"
	@echo "  $(BUILD_DIR)/mem_c"
	@echo "  $(BUILD_DIR)/uio_c"
	@echo "  $(BUILD_DIR)/mem_cpp"
	@echo "  $(BUILD_DIR)/uio_cpp"

# Send binaries to PYNQ-Z2
send: compile
	@echo "Sending binaries to PYNQ-Z2 board..."
	@scp $(BUILD_DIR)/mem_c $(BUILD_DIR)/uio_c $(BUILD_DIR)/mem_cpp $(BUILD_DIR)/uio_cpp pynq-z2-screen:/home/plnx

# Clean build directory
clean:
	@echo "Removing build directory..."
	@rm -rf $(BUILD_DIR)
