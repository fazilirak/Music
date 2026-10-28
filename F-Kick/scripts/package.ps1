# Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
# Dosyanın görevi: Ürün derleme, kaynak hazırlama ve dağıtım yardımcıları.
# Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
# Fazıl Irak · 27.10.2026 · Doğrulanmış yerel ikili/kaynak arşivi. Sistem kurulumu/yayın yapmaz.
$ErrorActionPreference = 'Stop'
$kickRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$kickVersion = '0.8.2'
$kickDist = Join-Path $kickRoot 'dist'
$kickBinary = Join-Path $kickDist "F-Kick-$kickVersion-Windows-x64"
$kickSource = Join-Path $kickDist "F-Kick-$kickVersion-Source"
$kickBuilt = Join-Path $kickRoot 'build\FKick_artefacts\Release'
if (!(Test-Path (Join-Path $kickRoot 'validation\TEST_SONUCLARI_TR.md'))) { throw 'Write validation report after tests before packaging.' }
if (!(Test-Path (Join-Path $kickBuilt 'VST3\F-Kick.vst3\Contents\x86_64-win\F-Kick.vst3'))) { throw 'Build VST3 first.' }
foreach ($kickExecutable in @('Standalone\F-Kick.exe','VST3\F-Kick.vst3\Contents\x86_64-win\F-Kick.vst3')) {
    if ((Get-Item -LiteralPath (Join-Path $kickBuilt $kickExecutable)).VersionInfo.ProductVersion -ne $kickVersion) { throw 'Stale Windows version resource; rebuild before packaging.' }
}
if (Test-Path $kickBinary) { throw 'Version folder exists; use a new version or review existing delivery.' }
New-Item -ItemType Directory -Force $kickDist, $kickBinary, $kickSource, (Join-Path $kickBinary 'VST3'), (Join-Path $kickBinary 'Standalone') | Out-Null
Copy-Item -LiteralPath (Join-Path $kickBuilt 'VST3\F-Kick.vst3') -Destination (Join-Path $kickBinary 'VST3') -Recurse
Copy-Item -LiteralPath (Join-Path $kickBuilt 'Standalone\F-Kick.exe') -Destination (Join-Path $kickBinary 'Standalone')
foreach ($kickName in @('README.md','LICENSE.txt','LICENSE','AUTHORS.md','source-lock.json','THIRD-PARTY-NOTICES.txt','licenses','docs','validation')) {
    Copy-Item -LiteralPath (Join-Path $kickRoot $kickName) -Destination $kickBinary -Recurse
}
foreach ($kickName in @('CMakeLists.txt','AGENTS.md','README.md','LICENSE.txt','LICENSE','AUTHORS.md','source-lock.json','THIRD-PARTY-NOTICES.txt','.gitignore','Source','Tests','scripts','Resources','licenses','docs','validation')) {
    Copy-Item -LiteralPath (Join-Path $kickRoot $kickName) -Destination $kickSource -Recurse
}
$kickDependencies = Join-Path $kickSource 'Dependencies\JUCE'
New-Item -ItemType Directory -Force $kickDependencies | Out-Null
$kickJuce = if (Test-Path (Join-Path $kickRoot 'Dependencies\JUCE\CMakeLists.txt')) { Join-Path $kickRoot 'Dependencies\JUCE' } else { [System.IO.Path]::GetFullPath((Join-Path $kickRoot '..\_Dependencies\JUCE')) }
& robocopy $kickJuce $kickDependencies /E /XD .git build .vs /XF '*.user' /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw 'JUCE source copy failed.' }
Add-Type -AssemblyName System.IO.Compression.FileSystem
foreach ($kickFolder in @($kickBinary,$kickSource)) {
    $kickZip = "$kickFolder.zip"
    if (Test-Path $kickZip) { throw "Archive already exists: $kickZip" }
    [System.IO.Compression.ZipFile]::CreateFromDirectory($kickFolder,$kickZip,[System.IO.Compression.CompressionLevel]::Optimal,$true)
}
Get-FileHash -Algorithm SHA256 (Join-Path $kickDist '*.zip') | ForEach-Object { "$($_.Hash.ToLowerInvariant())  $([System.IO.Path]::GetFileName($_.Path))" } | Set-Content -LiteralPath (Join-Path $kickDist 'SHA256SUMS.txt') -Encoding ascii
Get-ChildItem -LiteralPath $kickDist | Select-Object Name,Length

