#!/bin/bash

set -euo pipefail

./build_screen_test.sh

echo "Sending binary to PYNQ-Z2 board..."
scp build/screen_test pynqz2-screen:/home/petalinux