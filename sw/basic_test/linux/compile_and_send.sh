#!/bin/bash

set -euo pipefail

./build_screen_test.sh

echo "Sending binaries to PYNQ-Z2 board..."
scp build/screen_test_c build/screen_test_cpp pynqz2-screen:/home/petalinux