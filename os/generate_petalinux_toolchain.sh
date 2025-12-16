#!/bin/bash
set -euo pipefail

cd petalinux_screen

petalinux-build --sdk
petalinux-package sysroot --dir ../