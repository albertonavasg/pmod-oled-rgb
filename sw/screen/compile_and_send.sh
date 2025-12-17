#!/bin/bash

set -euo pipefail

make test_app

echo "Sending test_app to PYNQ-Z2 board..."
scp build/test_app pynqz2-screen:/home/petalinux