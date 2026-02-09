#!/bin/sh
set -euo pipefail

# Get the directory of the script
SCRIPT_DIR=$(dirname "$(realpath "$0")")
"$SCRIPT_DIR/../bin/service_app"