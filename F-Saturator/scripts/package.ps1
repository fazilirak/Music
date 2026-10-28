# Fazıl Irak tarafından değiştirildi: 23.10.2026
# SPDX-License-Identifier: AGPL-3.0-or-later
# Eski komut korunur; sürüm ve kaynak doğrulaması ortak paketleyicide yapılır.
$ErrorActionPreference = 'Stop'
$python = if ($env:FSATURATOR_PYTHON) { $env:FSATURATOR_PYTHON } else { 'python' }
& $python (Join-Path $PSScriptRoot 'package_distribution.py') --platform Windows
if ($LASTEXITCODE -ne 0) { throw 'F-Saturator packaging failed.' }
