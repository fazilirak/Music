# Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026
# Dosyanın görevi: Derlenmiş Windows dosyalarını proje içindeki dist dizinine kopyalamak.
# SPDX-License-Identifier: AGPL-3.0-or-later
$ErrorActionPreference = 'Stop'
$ftRoot = Split-Path -Parent $PSScriptRoot
$ftBuild = Join-Path $ftRoot 'build\FTransForm_artefacts\Release'
$ftDist = Join-Path $ftRoot 'dist'
# Burada sistem VST3 dizinine kurulum yapılmaz; kullanıcı test edeceği konuma kendisi kopyalar.
$ftExe = Join-Path $ftBuild 'Standalone\F-TransForm.exe'
$ftPlugin = Join-Path $ftBuild 'VST3\F-TransForm.vst3'
if (!(Test-Path -LiteralPath $ftExe) -or !(Test-Path -LiteralPath $ftPlugin)) {
    throw 'Once Windows Release derlemesini tamamlayin.'
}
New-Item -ItemType Directory -Force (Join-Path $ftDist 'Standalone'), (Join-Path $ftDist 'VST3') | Out-Null
Copy-Item -LiteralPath $ftExe -Destination (Join-Path $ftDist 'Standalone') -Force
Copy-Item -LiteralPath $ftPlugin -Destination (Join-Path $ftDist 'VST3') -Recurse -Force
# Dağıtım adı taşısa da bu klasör yerel prototiptir; genel yayın paketi hazırlamaz.
# Kaynak ve üçüncü taraf lisansları proje kökünde korunur.
$ftFiles = Get-ChildItem -LiteralPath (Join-Path $ftDist 'Standalone'),(Join-Path $ftDist 'VST3') -File -Recurse
$ftManifest = foreach ($ftFile in $ftFiles) {
    [ordered]@{ file = [IO.Path]::GetRelativePath($ftDist, $ftFile.FullName); sha256 = (Get-FileHash -LiteralPath $ftFile.FullName -Algorithm SHA256).Hash }
}
[ordered]@{ developer = 'Fazıl Irak'; version = '0.2.2'; platform = 'Windows x64'; files = @($ftManifest) } |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $ftDist 'LOCAL-BUILD.json') -Encoding utf8
Write-Output 'F-TransForm: dist/Standalone ve dist/VST3 hazir.'
