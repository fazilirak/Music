# F Saturator 0.3.0 — Mac derleme ve kurulum

Fazıl Irak tarafından oluşturuldu: 23.10.2026.

GitHub Actions, macos-15 üzerinde Apple Silicon ve Intel dilimlerini içeren
Universal VST3, AU ve bağımsız uygulama üretir. Dağıtım hedefi macOS 11.0'dır.
Kaynak ZIP'indeki vendor klasörü JUCE ve URW font kaynaklarını içerir.

Xcode Command Line Tools, Python 3.12+, CMake 3.24+ ve Ninja kurulu bir Mac üzerinde:

```sh
python3 -m pip install 'cmake>=3.24,<4' ninja
bash scripts/build-macos.sh
```

GitHub checkout'unda bootstrap betikleri sabit arşiv SHA-256 değerlerini doğrular.
Kaynak ZIP'inde aynı kaynaklar hazır olduğundan bootstrap onları yerinde doğrular.
Testler DSP kararlılığını, presetleri ve arayüzü sınar; tüm DAW ve Mac modellerinin
uyumluluğunu doğrulamaz. Özellikle ayrı Intel Mac ve gerçek DAW testleri gerekir.

Arşivi açın. VST3/F Saturator.vst3 paketini ~/Library/Audio/Plug-Ins/VST3/,
AU/F Saturator.component paketini ~/Library/Audio/Plug-Ins/Components/ içine kopyalayın.
DAW'da yeniden taratın. Bağımsız kullanım: Standalone/F Saturator.app.

Paket ad-hoc imzalıdır; Developer ID imzası ve Apple noter onayı yoktur.
Kaynak kod arşivi, lisanslar ve sürüm manifesti aynı GitHub sürümünde bulunur.
Dosyaların SHA-256 değerlerini platform RELEASE-MANIFEST.json ile karşılaştırın.
