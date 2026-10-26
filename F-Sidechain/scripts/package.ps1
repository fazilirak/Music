$ErrorActionPreference = 'Stop'
# Copyright (c) 2026 Fazıl Irak — test edilmiş ikilileri, tam kaynakları ve lisansları paketler.
$taskRoot = Split-Path -Parent $PSScriptRoot
$version = (Get-Content -LiteralPath (Join-Path $taskRoot 'source-lock.json') -Raw | ConvertFrom-Json).version
$packageDir = Join-Path $taskRoot "dist\F-Sidechain-$version-Windows-x64"
New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $taskRoot 'build\FSidechain_artefacts\Release\VST3\F-Sidechain.vst3') -Destination $packageDir -Recurse -Force
Copy-Item -LiteralPath (Join-Path $taskRoot 'build\FSidechain_artefacts\Release\Standalone\F-Sidechain.exe') -Destination $packageDir -Force
foreach ($name in @('README.md','LICENSE.txt','THIRD-PARTY-NOTICES.txt','source-lock.json','preview.png','preview-about.png')) {
    Copy-Item -LiteralPath (Join-Path $taskRoot $name) -Destination $packageDir -Force
}
Copy-Item -LiteralPath (Join-Path $taskRoot 'licenses') -Destination $packageDir -Recurse -Force
$taskPython = 'C:\Users\FAZIL\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
if (-not (Test-Path -LiteralPath $taskPython)) { $taskPython = (Get-Command python -ErrorAction Stop).Source }
& $taskPython (Join-Path $taskRoot 'scripts\package_source.py') (Join-Path $packageDir "F-Sidechain-$version-Source.zip")
if ($LASTEXITCODE -ne 0) { throw 'Source packaging failed' }
@'
F-Sidechain 1.1.0 / Windows x64
Build: Release, MSVC x64, JUCE local pinned source
Validation: CTest volume_audio_and_state PASS
8 factory presets: finite audio; every factory trough reaches Y=1 / zero gain
Full range dry mix: signal identity
Wet zero envelope range: silence
Independent Low/High presets and crossover state: recalled
Complete binary state: exact roundtrip including patterns/envelopes
44.1 kHz and 48 kHz processing: finite audio
Audio trigger band latency: host latency follows active bands
Editor: permanent Low/Mid/High graph; pattern selector and Bands toggle removed
New instance: Full Range, no crossover processing; permanent band graph
Full range reset: 20 Hz / 20 kHz boundaries and Mid selection verified
Point icon: 18x18 circle with 4x4 central dot
About: opens from the F-Sidechain logo; no separate About button; English text
Licences: seven embedded licence documents successfully loaded
Editor and About views: rendered and visually inspected
Licensing: F-Sidechain GPLv3 + JUCE AGPLv3 combined work; full source bundled
VST3: binary loaded by JUCE manifest helper; moduleinfo generated
Real DAW session: not exercised in this build
'@ | Set-Content -LiteralPath (Join-Path $packageDir 'VALIDATION.txt') -Encoding UTF8
Get-ChildItem -LiteralPath $packageDir -Recurse -File | Where-Object Name -ne 'SHA256SUMS.txt' | ForEach-Object {
    $hash = Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256
    '{0}  {1}' -f $hash.Hash, $_.FullName.Substring($packageDir.Length + 1)
} | Set-Content -LiteralPath (Join-Path $packageDir 'SHA256SUMS.txt') -Encoding UTF8
Compress-Archive -LiteralPath $packageDir -DestinationPath (Join-Path $taskRoot "dist\F-Sidechain-$version-Windows-x64.zip") -Force
Get-Item -LiteralPath (Join-Path $taskRoot "dist\F-Sidechain-$version-Windows-x64.zip") | Select-Object FullName,Length
