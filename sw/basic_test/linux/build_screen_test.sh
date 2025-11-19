#!/bin/bash

set -euo pipefail

TOOLCHAIN_PATH="../../../os/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-linux-gnueabihf/bin"
CC="${TOOLCHAIN_PATH}/arm-none-linux-gnueabihf-gcc"
CXX="${TOOLCHAIN_PATH}/arm-none-linux-gnueabihf-g++"

BUILD_DIR="$PWD/build"
mkdir -p "$BUILD_DIR"

echo "Compiling Screen Test program in C..."
$CC mem.c -o $BUILD_DIR/mem_c
$CC uio.c -o $BUILD_DIR/uio_c

echo "Compoling Screen Test program in C++..."
$CXX mem.cpp -o $BUILD_DIR/mem_cpp
$CXX uio.cpp -o $BUILD_DIR/uio_cpp

echo "Compilation finished"
echo "Generated binary: build/mem_c"
echo "Generated binary: build/uio_c"
echo "Generated binary: build/mem_cpp"
echo "Generated binary: build/uio_cpp"
