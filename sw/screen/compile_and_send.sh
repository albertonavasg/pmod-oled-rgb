#!/bin/bash

set -euo pipefail

make screen_test

echo "Sending screen_test to PYNQ-Z2 board..."
scp build/screen_test pynqz2-screen:/home/petalinux