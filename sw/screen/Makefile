# Paths
SRC_DIR := src
INCLUDE_DIR := include
APP_DIR := apps
BUILD_DIR := build

# Makefile silent
.SILENT:

# Cross-compiler
TOOLCHAIN_PATH := $(HOME)/tools/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin
CXX := $(TOOLCHAIN_PATH)/arm-none-linux-gnueabihf-g++
CXXFLAGS := -I$(INCLUDE_DIR) -Wall -O2 -std=c++20

# Sources
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default: all
.DEFAULT_GOAL := all

# Autodetect apps and create targets
APPS := $(basename $(notdir $(wildcard $(APP_DIR)/*.cpp)))
APP_TARGETS := $(addprefix $(BUILD_DIR)/,$(APPS))

# All apps target
all: $(APP_TARGETS)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile src files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile app files
$(BUILD_DIR)/%.o: $(APP_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link apps
$(BUILD_DIR)/%: $(OBJS) $(BUILD_DIR)/%.o
	echo "Linking -> $@"
	$(CXX) $(CXXFLAGS) $^ -o $@
	echo "Built -> $@"

# Make each app a phony target to force Make to expand pattern rules
.PHONY: $(APPS)
$(APPS): %: $(BUILD_DIR)/%

# Clean
.PHONY: clean all
clean:
	rm -rf $(BUILD_DIR)/*
	echo "Clean done"
