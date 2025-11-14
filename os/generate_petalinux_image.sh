#!/bin/bash
set -euo pipefail

cd petalinux_screen

petalinux-config --get-hw-description screen.xsa --silentconfig
petalinux-build
petalinux-package boot --u-boot --fpga --force