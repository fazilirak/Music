#!/usr/bin/env bash
# Fazıl Irak — 26.10.2026: macOS Universal derleme ve test.
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build-macos -G Ninja -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-macos --parallel 3
ctest --test-dir build-macos --output-on-failure --timeout 300
python3 scripts/package_distribution.py --macos
