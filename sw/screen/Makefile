# Paths
SRC_DIR     := src
APP_DIR     := apps
INCLUDE_DIR := include
BIN_DIR     := bin

# Cross-compiler
TOOLCHAIN_PATH := $(HOME)/tools/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin
CXX := $(TOOLCHAIN_PATH)/arm-none-linux-gnueabihf-g++

# Flags
CXXFLAGS := -I$(INCLUDE_DIR) -isystem $(INCLUDE_DIR)/nlohmann -Wall -O2 -std=c++20 -Wno-psabi
LDFLAGS :=

# Common sources and corresponding object files
COMMON_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(COMMON_SRCS))

# App object names
TEST_APP_OBJ    := $(BIN_DIR)/test_app.o
SERVICE_APP_OBJ := $(BIN_DIR)/service_app.o

# App binary names
TEST_APP_BIN    := $(BIN_DIR)/test_app
SERVICE_APP_BIN := $(BIN_DIR)/service_app

# Makefile silent
.SILENT:
# Explicit default goal
.DEFAULT_GOAL := all

# Main targets
all: test_app service_app

test_app: $(TEST_APP_BIN)

service_app: $(SERVICE_APP_BIN)

clean:
	echo "[CLEAN]"
	rm -rf $(BIN_DIR)

.PHONY: all clean test_app service_app

# Utility targets
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile common sources
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	echo "[CXX] $(notdir $<)"
	$(CXX) $(CXXFLAGS) -c -o $< $@

# Compile each app
$(TEST_APP_OBJ): $(APP_DIR)/test_app.cpp | $(BIN_DIR)
	echo "[APP] $(notdir $<)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(SERVICE_APP_OBJ): $(APP_DIR)/service_app.cpp | $(BIN_DIR)
	echo "[APP] $(notdir $<)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Link each app
$(TEST_APP_BIN): $(COMMON_OBJS) $(TEST_APP_OBJ)
	echo "[LD] $(notdir $@)"
	$(CXX) $(LDFLAGS) -o $@ $(COMMON_OBJS) $(TEST_APP_OBJ)

$(SERVICE_APP_BIN): $(COMMON_OBJS) $(SERVICE_APP_OBJ)
	echo "[LD] $(notdir $@)"
	$(CXX) $(LDFLAGS) -o $@ $(COMMON_OBJS) $(SERVICE_APP_OBJ)
