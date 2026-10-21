# Dağıtım aracı: derlenmiş Windows dosyalarını, belgeleri ve kaynak arşivini bir araya getirir.
# Önce build-windows.cmd ile derleme ve testleri başarıyla tamamlayın.
$ErrorActionPreference = 'Stop'
# Yollar betiğin konumundan türetilir; çalıştırıldığı PowerShell klasörüne bağlı değildir.
$projectRoot = Split-Path -Parent $PSScriptRoot
$delivery = Join-Path $projectRoot 'dist'
$artefacts = Join-Path $projectRoot 'build/IRAK_artefacts/Release'
$factory = Join-Path $delivery 'FactoryPresets'
# Eksik preset bankasıyla paket üretmeyi önler; test uygulaması tam 16 preset dışarı aktarır.
if (@(Get-ChildItem -LiteralPath $factory -Filter '*.irakpreset').Count -ne 16) { throw 'Run verification to export sixteen factory presets.' }
# Sürüm 0.2.0 sabit olarak kullanılıyor. Yeni sürümde CMake, bu adlar ve dağıtım belgelerini birlikte güncelleyin.
$source = Join-Path $delivery 'Source-0.2.0'
New-Item -ItemType Directory -Force -Path (Join-Path $delivery 'Standalone'),(Join-Path $delivery 'VST3'),(Join-Path $source 'Research') | Out-Null
# Yalnız ürün EXE/VST3 dosyaları kopyalanır; build önbelleği ve üçüncü taraf araştırma depoları dağıtılmaz.
Copy-Item -LiteralPath (Join-Path $artefacts 'Standalone/F Saturator.exe') -Destination (Join-Path $delivery 'Standalone/F Saturator.exe') -Force
Copy-Item -LiteralPath (Join-Path $artefacts 'VST3/F Saturator.vst3') -Destination (Join-Path $delivery 'VST3') -Recurse -Force
# Kaynak paketi açık kaynak dağıtımın parçasıdır; kendi kodu ve derleme tarifleri birlikte bulunur.
foreach ($file in @('README.md','LICENSE','CMakeLists.txt','GELISTIRME-TR.md','REVIEW-TR.md')) { Copy-Item -LiteralPath (Join-Path $projectRoot $file) -Destination $source -Force }
foreach ($folder in @('Source','Tests','scripts','Resources')) { Copy-Item -LiteralPath (Join-Path $projectRoot $folder) -Destination $source -Recurse -Force }
foreach ($file in @('DSP-RESEARCH.md','dependencies.json')) { Copy-Item -LiteralPath (Join-Path $projectRoot "Research/$file") -Destination (Join-Path $source 'Research') -Force }
# JUCE lisans bildirimi ayrıca eklenir. Kullanılan kesin JUCE commit’i Research/dependencies.json içindedir.
$juceLicense = Join-Path $projectRoot '../Shaper/upstream/gate12/libs/JUCE/LICENSE.md'
if (-not (Test-Path -LiteralPath $juceLicense)) { $juceLicense = Join-Path $projectRoot '../upstream/gate12/libs/JUCE/LICENSE.md' }
Copy-Item -LiteralPath $juceLicense -Destination (Join-Path $delivery 'JUCE-LICENSE.md') -Force
Copy-Item -LiteralPath (Join-Path $delivery 'JUCE-LICENSE.md') -Destination $source -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'README.md'),(Join-Path $projectRoot 'LICENSE') -Destination $delivery -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'build/Testing/Temporary/LastTest.log') -Destination (Join-Path $delivery 'test-results.txt') -Force
# Önce karşılık gelen kaynak ZIP’i, ardından onu da içeren Windows ZIP’i oluşturulur.
# Kaynak dosyasını Windows paketinden çıkarmak yerine lisans/dağıtım tercihini birlikte değerlendirin.
Compress-Archive -Path (Join-Path $source '*') -DestinationPath (Join-Path $delivery 'F-Saturator-0.2.0-Source.zip') -Force
Compress-Archive -Path (Join-Path $delivery 'Standalone'),(Join-Path $delivery 'VST3'),$factory,(Join-Path $delivery 'Auditions'),(Join-Path $delivery 'README.md'),(Join-Path $delivery 'LICENSE'),(Join-Path $delivery 'JUCE-LICENSE.md'),(Join-Path $delivery 'DSP-REPORT.md'),(Join-Path $delivery 'F-Saturator-preview.png'),(Join-Path $delivery 'F-Saturator-0.2.0-Source.zip'),(Join-Path $delivery 'test-results.txt') -DestinationPath (Join-Path $delivery 'F-Saturator-0.2.0-Windows-x64.zip') -Force
