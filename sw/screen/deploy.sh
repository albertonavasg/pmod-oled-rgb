#!/bin/sh
set -e

TARGET_USER=root
TARGET_HOST=pynqz2-screen
TARGET_DIR=/opt/screen

BIN_DIR=build
ASSETS_DIR=assets

echo "=== Building ==="
make test_app service_app

echo "=== Checking local artifacts ==="
[ -x "$BIN_DIR/test_app" ] || { echo "test_app missing"; exit 1; }
[ -x "$BIN_DIR/service_app" ] || { echo "service_app missing"; exit 1; }
[ -f "$ASSETS_DIR/config.json" ] || { echo "config.json missing"; exit 1; }
[ -d "$ASSETS_DIR/images" ] || { echo "images directory missing"; exit 1; }

echo "=== Checking target directory ==="
ssh "$TARGET_USER@$TARGET_HOST" "mkdir -p $TARGET_DIR/bin $TARGET_DIR/assets"

echo "=== Deploying binaries ==="
rsync -avz --delete \
    "$BIN_DIR/test_app" \
    "$BIN_DIR/service_app" \
    "$TARGET_USER@$TARGET_HOST:$TARGET_DIR/bin/"

echo "=== Deploying assets ==="
rsync -avz --delete \
    "$ASSETS_DIR/" \
    "$TARGET_USER@$TARGET_HOST:$TARGET_DIR/assets/"

echo "=== Verifying on target ==="
ssh "$TARGET_USER@$TARGET_HOST" << EOF
set -e
[ -x "$TARGET_DIR/bin/test_app" ]
[ -x "$TARGET_DIR/bin/service_app" ]
[ -f "$TARGET_DIR/assets/config.json" ]
[ -d "$TARGET_DIR/assets/images" ]
echo "Deployment OK"
EOF
