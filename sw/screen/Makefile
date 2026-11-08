# Directories
SRC_DIR := src
APP_DIR := apps
INCLUDE_DIR := include
BUILD_DIR := build
SCRIPTS_DIR := scripts
ASSETS_DIR := assets

# Cross-compiler
TOOLCHAIN_PATH := $(HOME)/tools/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin
CXX := $(TOOLCHAIN_PATH)/arm-none-linux-gnueabihf-g++

# Compiler flags
CXXFLAGS := -I$(INCLUDE_DIR) -isystem $(INCLUDE_DIR)/nlohmann -Wall -O2 -std=c++20 -Wno-psabi

# Target configuration
TARGET_HOST := pynq-z2-screen
TARGET_USER := root
TARGET_DIR := /opt/screen

# Common source files
COMMON_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMMON_SRCS))

# Application objects
TEST_APP_OBJ := $(BUILD_DIR)/test_app.o
SERVICE_APP_OBJ := $(BUILD_DIR)/service_app.o

# Application binaries
TEST_APP_BIN := $(BUILD_DIR)/test_app
SERVICE_APP_BIN := $(BUILD_DIR)/service_app


# Main targets
.PHONY: all compile test_app service_app send deploy check_local clean

all: compile send

compile: test_app service_app
	@echo "Compilation finished"

# Build applications
test_app: $(TEST_APP_BIN)

service_app: $(SERVICE_APP_BIN)


# Compile common sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test application
$(TEST_APP_OBJ): $(APP_DIR)/test_app.cpp | $(BUILD_DIR)
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile service application
$(SERVICE_APP_OBJ): $(APP_DIR)/service_app.cpp | $(BUILD_DIR)
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@


# Link applications
$(TEST_APP_BIN): $(COMMON_OBJS) $(TEST_APP_OBJ)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(COMMON_OBJS) $(TEST_APP_OBJ)

$(SERVICE_APP_BIN): $(COMMON_OBJS) $(SERVICE_APP_OBJ)
	@echo "[LD] $@"
	@$(CXX) -o $@ $(COMMON_OBJS) $(SERVICE_APP_OBJ)


# Create binary directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)


# Send binaries to PYNQ-Z2
send: compile
	@echo "Sending applications to PYNQ-Z2 board..."
	@scp $(TEST_APP_BIN) $(SERVICE_APP_BIN) $(TARGET_HOST):$(TARGET_DIR)/bin


# Check local deployment artifacts
check_local:
	@echo "Checking local artifacts..."
	@test -x $(TEST_APP_BIN) || { echo "test_app missing"; exit 1; }
	@test -x $(SERVICE_APP_BIN) || { echo "service_app missing"; exit 1; }
	@test -x $(SCRIPTS_DIR)/run_service.sh || { echo "run_service.sh missing"; exit 1; }
	@test -f $(ASSETS_DIR)/config.json || { echo "config.json missing"; exit 1; }
	@test -d $(ASSETS_DIR)/images || { echo "images directory missing"; exit 1; }


# Complete deployment
deploy: compile check_local
	@echo "Checking target directory..."
	@ssh $(TARGET_USER)@$(TARGET_HOST) "mkdir -p $(TARGET_DIR)/bin $(TARGET_DIR)/scripts $(TARGET_DIR)/assets"

	@echo "Deploying binaries..."
	@rsync -avz --delete $(TEST_APP_BIN) $(SERVICE_APP_BIN) $(TARGET_USER)@$(TARGET_HOST):$(TARGET_DIR)/bin/

	@echo "Deploying scripts..."
	@rsync -avz --delete $(SCRIPTS_DIR)/ $(TARGET_USER)@$(TARGET_HOST):$(TARGET_DIR)/scripts/

	@echo "Deploying assets..."
	@rsync -avz --delete $(ASSETS_DIR)/ $(TARGET_USER)@$(TARGET_HOST):$(TARGET_DIR)/assets/

	@echo "Verifying deployment..."
	@ssh $(TARGET_USER)@$(TARGET_HOST) "set -e; \
		test -x $(TARGET_DIR)/bin/test_app; \
		test -x $(TARGET_DIR)/bin/service_app; \
		test -x $(TARGET_DIR)/scripts/run_service.sh; \
		test -f $(TARGET_DIR)/assets/config.json; \
		test -d $(TARGET_DIR)/assets/images; \
		echo 'Deployment OK'"


# Clean build directory
clean:
	@echo "Removing build directory..."
	@rm -rf $(BUILD_DIR)