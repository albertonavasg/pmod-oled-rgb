#!/bin/bash

set -euo pipefail

TOOLCHAIN_PATH="../../../os/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-linux-gnueabihf/bin"
CC="${TOOLCHAIN_PATH}/arm-none-linux-gnueabihf-gcc"
CXX="${TOOLCHAIN_PATH}/arm-none-linux-gnueabihf-g++"

BUILD_DIR="$PWD/build"
mkdir -p "$BUILD_DIR"

echo "Compiling Screen Test program in C..."
$CC screen_test.c -o $BUILD_DIR/screen_test_c

echo "Compoling Screen Test program in C++..."
$CXX screen_test.cpp -o $BUILD_DIR/screen_test_cpp

echo "Compilation finished"
echo "Generated binary: build/screen_test_c"
echo "Generated binary: build/screen_test_cpp"