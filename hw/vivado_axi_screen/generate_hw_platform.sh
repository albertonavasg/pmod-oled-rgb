#!/usr/bin/env bash

set -euo pipefail

vivado -mode batch -source hw_platform.tcl
