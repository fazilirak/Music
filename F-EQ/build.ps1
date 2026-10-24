# Türkçe bakım açıklamaları: Fazıl Irak
# Son düzenleme: 22.10.2026
# Görev: Rust toolchain/Cargo kaynağını çözer; test, preview, plugin ve standalone çıktılarını hazırlar.
param([switch]$Test, [switch]$PackageSource, [switch]$Offline)
$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath $PSScriptRoot
try {
    # Music içindeki ortak Rust araçları ve bu ürüne ait paket önbelleği.
    $musicRust = Join-Path (Split-Path -Parent $PSScriptRoot) '_Tools/Rust'
    $toolchainBin = Join-Path $musicRust 'rustup/toolchains/1.97.1-x86_64-pc-windows-msvc/bin'
    if(Test-Path -LiteralPath (Join-Path $toolchainBin 'cargo.exe')) {
        $env:PATH = $toolchainBin + ';' + $env:PATH
        $env:RUSTUP_HOME = Join-Path $musicRust 'rustup'
        $env:CARGO_HOME = Join-Path (Split-Path -Parent $PSScriptRoot) '_Tools/F-EQ-Cargo'
    }
    $cargo = Get-Command cargo -ErrorAction SilentlyContinue
    if ($cargo) { $cargoExe = $cargo.Source }
    else {
        $modelTools = Join-Path (Split-Path -Parent $PSScriptRoot) '_Tools/Rust'
        $cargoExe = Join-Path $modelTools 'rustup/toolchains/1.97.1-x86_64-pc-windows-msvc/bin/cargo.exe'
        if (!(Test-Path -LiteralPath $cargoExe)) { throw 'Install Rust 1.97.1 and Visual Studio C++ Build Tools, or add cargo to PATH.' }
        $env:RUSTUP_HOME = Join-Path $modelTools 'rustup'
    }
    if (Test-Path -LiteralPath '.tools/cargo/registry') { $env:CARGO_HOME = Join-Path $PSScriptRoot '.tools/cargo' }
    if (!$env:CARGO_TARGET_DIR) { $env:CARGO_TARGET_DIR = Join-Path $PSScriptRoot 'target' }
    function Cargo-Step([string[]]$Arguments) {
        if ($Offline) { $Arguments += '--offline' }
        & $cargoExe @Arguments
        if ($LASTEXITCODE -ne 0) { throw "Cargo failed: $($Arguments -join ' ')" }
    }
    if ($Test) { Cargo-Step -Arguments @('test','--release','--locked') }
    Cargo-Step -Arguments @('run','--example','preview','--release','--locked')
    Cargo-Step -Arguments @('build','--lib','--release','--locked')
    Cargo-Step -Arguments @('build','--example','simple-eq','--features','standalone','--release','--locked')
    $package = Join-Path $PSScriptRoot 'dist/F-EQ-Windows-0.1.10'
    $vst = Join-Path $package 'F EQ.vst3/Contents/x86_64-win'
    New-Item -ItemType Directory -Force -Path $vst | Out-Null
    $dll = Join-Path $env:CARGO_TARGET_DIR 'release/simple_eq.dll'
    Copy-Item -LiteralPath $dll -Destination (Join-Path $vst 'F EQ.vst3') -Force
    Copy-Item -LiteralPath $dll -Destination (Join-Path $package 'F EQ.clap') -Force
    Copy-Item -LiteralPath (Join-Path $env:CARGO_TARGET_DIR 'release/examples/simple-eq.exe') -Destination (Join-Path $package 'F EQ.exe') -Force
    foreach ($file in @('README.md','LICENSE','THIRD-PARTY-NOTICES.txt','Luftikus-LICENSE.txt')) { Copy-Item -LiteralPath $file -Destination $package -Force }
    New-Item -ItemType Directory -Force -Path (Join-Path $package 'docs') | Out-Null
    Copy-Item -LiteralPath 'docs/panel.png' -Destination (Join-Path $package 'docs/panel.png') -Force
    Copy-Item -LiteralPath 'docs/analyzer-preview.png' -Destination (Join-Path $package 'docs/analyzer-preview.png') -Force
    Copy-Item -LiteralPath 'docs/about.png' -Destination (Join-Path $package 'docs/about.png') -Force
    Compress-Archive -LiteralPath $package -DestinationPath 'dist/F-EQ-Windows-0.1.10.zip' -Force
    if ($PackageSource) {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $sourceZip = Join-Path $PSScriptRoot 'dist/F-EQ-Source-0.1.10.zip'
        # Create/overwrite one known output, without touching the source tree.
        $stream = [IO.File]::Open($sourceZip,[IO.FileMode]::Create)
        $zip = [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create)
        try {
            $items = @('src','examples','tests','third_party','.cargo','docs','Cargo.toml','Cargo.lock','rust-toolchain.toml','build.ps1','README.md','LICENSE','THIRD-PARTY-NOTICES.txt','Luftikus-LICENSE.txt','.gitignore')
            foreach ($item in $items) {
                $files = if (Test-Path -LiteralPath $item -PathType Container) { Get-ChildItem -LiteralPath $item -Recurse -File -Force } else { Get-Item -LiteralPath $item }
                foreach ($file in $files) {
                    $rel = [IO.Path]::GetRelativePath($PSScriptRoot,$file.FullName).Replace('\','/')
                    if ($rel -match '(^|/)\.git(/|$)' -or $rel -match '(^|/)target(/|$)' -or $rel -match '^docs/.*-log\.txt$') { continue }
                    [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$file.FullName,$rel,[IO.Compression.CompressionLevel]::Optimal) | Out-Null
                }
            }
        } finally { $zip.Dispose(); $stream.Dispose() }
    }
    Get-ChildItem -LiteralPath 'dist' -Filter '*.zip' | ForEach-Object { $h=Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256; "$($h.Hash.ToLower())  $($_.Name)" } | Set-Content -LiteralPath 'dist/SHA256SUMS.txt' -Encoding ascii
    Write-Output "F EQ ready: $package"
} finally { Pop-Location }

