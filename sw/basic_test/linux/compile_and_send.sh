#!/bin/bash

set -euo pipefail

./build_screen_test.sh

echo "Sending binaries to PYNQ-Z2 board..."
scp build/mem_c build/uio_c build/mem_cpp build/uio_cpp pynqz2-screen:/home/petalinux