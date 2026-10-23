#!/usr/bin/env bash
# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: AGPL-3.0-or-later
set -euo pipefail
cd "$(dirname "$0")/.."
[[ "$(uname -s)" == Darwin ]] || { echo 'macOS is required.' >&2; exit 1; }
xcode-select -p
python3 scripts/bootstrap_juce.py
python3 scripts/bootstrap_fonts.py
cmake -S . -B build-macos -G Ninja -DCMAKE_BUILD_TYPE=Release \
  '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-macos --parallel "${FSATURATOR_BUILD_JOBS:-3}"
ctest --test-dir build-macos --output-on-failure --timeout 300
python3 scripts/package_distribution.py --platform macOS
