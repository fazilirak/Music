#!/usr/bin/env bash
# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "This build requires macOS and Xcode Command Line Tools." >&2
    exit 1
fi
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"
PYTHON="${SHAPER_PYTHON:-python3}"
BUILD_DIR="build-macos"
xcrun --find clang++ >/dev/null
command -v cmake >/dev/null
command -v ninja >/dev/null
cmake -S . -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DPython3_EXECUTABLE="$(command -v "$PYTHON")" \
    -DSHAPER_BUILD_TESTS=ON
cmake --build "$BUILD_DIR" --parallel "${SHAPER_BUILD_JOBS:-3}"
ctest --test-dir "$BUILD_DIR" --output-on-failure --timeout 300
"$PYTHON" scripts/package_macos.py
