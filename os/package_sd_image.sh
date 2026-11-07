#!/usr/bin/env bash

set -euo pipefail

IMAGE_DIR="petalinux_screen/images/linux"
SD_DIR="/media/$USER"
ROOTFS_DIR="$SD_DIR/rootfs"
BOOT_DIR="$SD_DIR/BOOT"

echo "Copy BOOT files"
cp $IMAGE_DIR/BOOT.BIN $BOOT_DIR
cp $IMAGE_DIR/image.ub $BOOT_DIR
cp $IMAGE_DIR/boot.scr $BOOT_DIR

echo "Extracting root filesystem"
sudo tar -xzvf $IMAGE_DIR/rootfs.tar.gz -C $ROOTFS_DIR
sync

