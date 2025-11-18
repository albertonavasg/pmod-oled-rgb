#!/bin/bash

set -euo pipefail

TOOLCHAIN_PATH="../../../os/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin"
CC="${TOOLCHAIN_PATH}/arm-linux-gnueabihf-gcc"

BUILD_DIR="$PWD/build"
mkdir -p "$BUILD_DIR"

echo "Compiling Screen Test program..."
$CC screen_test.c -o $BUILD_DIR/screen_test

echo "Compilation finished"
echo "Generated binary: build/screen_test"