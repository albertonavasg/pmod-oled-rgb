#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
XSA="$SCRIPT_DIR/../hw/vivado_axi_screen/build/screen.xsa"

cd "$SCRIPT_DIR/petalinux_screen"

petalinux-config --get-hw-description "$XSA" --silentconfig
petalinux-build
petalinux-package boot --u-boot --fpga --force
