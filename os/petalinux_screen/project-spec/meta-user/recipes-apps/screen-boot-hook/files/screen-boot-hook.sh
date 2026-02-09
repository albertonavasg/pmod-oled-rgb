#!/bin/sh

SERVICE_SCRIPT=/opt/screen/scripts/run_service.sh

if [ -x "$SERVICE_SCRIPT" ]; then
    echo "Screen Boot Hook: executing run_service.sh"
    exec "$SERVICE_SCRIPT"
else
    echo "Screen Boot Hook: run_service.sh not found or not executable"
fi