# Türkçe bakım açıklamaları: Fazıl Irak
# Son düzenleme: 26.10.2026
# Görev: Rust toolchain/Cargo kaynağını çözer; test, preview, plugin ve standalone çıktılarını hazırlar.
param([switch]$Test, [switch]$PackageSource, [switch]$Offline)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
Push-Location -LiteralPath $projectRoot
try {
    # Music içindeki ortak Rust araçları ve bu ürüne ait paket önbelleği.
    $musicRust = Join-Path (Split-Path -Parent $PSScriptRoot) '_Tools/Rust'
    $toolchainBin = Join-Path $musicRust 'rustup/toolchains/1.97.1-x86_64-pc-windows-msvc/bin'
    if(Test-Path -LiteralPath (Join-Path $toolchainBin 'cargo.exe')) {
        $env:PATH = $toolchainBin + ';' + $env:PATH
        $env:RUSTUP_HOME = Join-Path $musicRust 'rustup'
        $env:CARGO_HOME = Join-Path $musicRust 'cargo'
    }
    $cargoCommand = Get-Command cargo -ErrorAction SilentlyContinue
    if ($cargoCommand) { $cargoExe = $cargoCommand.Source }
    else {
        $localTools = Join-Path (Split-Path -Parent $projectRoot) '_Tools/Rust'
        $cargoExe = Join-Path $localTools 'cargo/bin/cargo.exe'
        if (!(Test-Path -LiteralPath $cargoExe)) { throw 'Rust is required. Install Rust 1.97.1 and Visual Studio C++ Build Tools.' }
        $env:CARGO_HOME = Join-Path $localTools 'cargo'
        $env:RUSTUP_HOME = Join-Path $localTools 'rustup'
    }
    if (!$env:CARGO_TARGET_DIR) { $env:CARGO_TARGET_DIR = Join-Path $projectRoot 'target' }
    # Keep machine paths out of shipped binaries; link the C runtime statically.
    $rustOptions = @('-C', 'target-feature=+crt-static', "--remap-path-prefix=$projectRoot=Model-F", "--remap-path-prefix=$env:CARGO_TARGET_DIR=target")
    if ($env:CARGO_HOME) { $rustOptions += "--remap-path-prefix=$env:CARGO_HOME=.cargo" }
    $env:CARGO_ENCODED_RUSTFLAGS = $rustOptions -join [char]31
    function Cargo-Step([string[]]$Arguments) {
        if($Offline){$Arguments += '--offline'}
        & $cargoExe @Arguments
        if ($LASTEXITCODE -ne 0) { throw "Cargo failed: $($Arguments -join ' ')" }
    }
    if ($Test) {
        Cargo-Step -Arguments @('test', '-p', 'model-f', '--release', '--locked')
        Cargo-Step -Arguments @('test', '-p', 'model-f-core', '--test', 'supersaw', '--release', '--locked')
        Cargo-Step -Arguments @('run', '-p', 'model-f', '--example', 'supersaw-load', '--release', '--locked', '--', 'docs/supersaw-load.json')
        Cargo-Step -Arguments @('run', '-p', 'model-f', '--example', 'supersaw-load', '--release', '--locked', '--', 'docs/mseg-filter-load.json', '1')
        Cargo-Step -Arguments @('run', '-p', 'model-f', '--example', 'supersaw-load', '--release', '--locked', '--', 'docs/mseg-volume-load.json', '3')
        Cargo-Step -Arguments @('run', '-p', 'model-f', '--example', 'render-bank', '--release', '--locked', '--', 'docs/audio')
    }
    Cargo-Step -Arguments @('run', '-p', 'model-f', '--example', 'preview', '--release', '--locked', '--', 'docs')
    Cargo-Step -Arguments @('build', '-p', 'model-f', '--lib', '--release', '--locked')
    $dist = Join-Path $projectRoot 'dist'
    $package = Join-Path $dist 'Model-F-Windows-0.2.7'
    $vst = Join-Path $package 'Model F.vst3/Contents/x86_64-win'
    New-Item -ItemType Directory -Force -Path $vst | Out-Null
    $library = Join-Path $env:CARGO_TARGET_DIR 'release/model_f.dll'
    Copy-Item -LiteralPath $library -Destination (Join-Path $vst 'Model F.vst3') -Force
    Copy-Item -LiteralPath $library -Destination (Join-Path $package 'Model F.clap') -Force
    Cargo-Step -Arguments @('build', '-p', 'model-f', '--example', 'model-f-standalone', '--features', 'standalone', '--release', '--locked')
    Copy-Item -LiteralPath (Join-Path $env:CARGO_TARGET_DIR 'release/examples/model-f-standalone.exe') -Destination (Join-Path $package 'Model F.exe') -Force
    foreach ($file in @('README.md','LICENSE','THIRD-PARTY-NOTICES.txt')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot $file) -Destination $package -Force
    }
    Copy-Item -LiteralPath (Join-Path $projectRoot 'docs') -Destination $package -Recurse -Force
    Compress-Archive -LiteralPath $package -DestinationPath (Join-Path $dist 'Model-F-Windows-0.2.7.zip') -Force
    if ($PackageSource) {
        $sourceItems = @('crates','third_party','xtask','.cargo','docs','Cargo.toml','Cargo.lock','rust-toolchain.toml','rustfmt.toml','clippy.toml','bundler.toml','build.ps1','generate-factory.cjs','generate-factory.py','README.md','LICENSE','THIRD-PARTY-NOTICES.txt') | ForEach-Object { Join-Path $projectRoot $_ }
        Compress-Archive -LiteralPath $sourceItems -DestinationPath (Join-Path $dist 'Model-F-Source-0.2.7.zip') -Force
    }
    Write-Output "Model F is ready: $package"
} finally { Pop-Location }
