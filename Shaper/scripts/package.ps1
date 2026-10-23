# Fazıl Irak tarafından oluşturuldu / değiştirildi: 23.10.2026
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (c) 2026 Fazıl Irak
param([string]$Python = "python")
$ErrorActionPreference = 'Stop'
& $Python (Join-Path $PSScriptRoot 'package_distribution.py')
if ($LASTEXITCODE -ne 0) { throw 'Distribution packaging failed.' }
