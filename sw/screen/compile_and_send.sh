#!/bin/bash

set -euo pipefail

make test_app service_app

echo "Sending apps to PYNQ-Z2 board..."
scp build/test_app build/service_app pynqz2-screen:/opt/screen/bin