# Shaper macOS derlemesi

Fazıl Irak — 23.10.2026

Hedefler: Apple Silicon ve Intel için ortak arm64/x86_64 Standalone `.app`,
VST3 `.vst3` ve Audio Unit `.component`. Minimum sistem hedefi macOS 11.0.
Bu hedefler gerçek macOS derlemesi ve host testleri yerine geçmez.

## Mac üzerinde

Xcode Command Line Tools, CMake 3.24 veya üzeri, Ninja ve Python 3 gerekir.
Kaynak klasöründe `bash scripts/build-macos.sh` çalıştırın. Script universal
derleme yapar, dört CTest hedefini çalıştırır ve çıktıları paketler.
`SHAPER_BUILD_JOBS` paralel iş sayısını, `SHAPER_PYTHON` Python yolunu belirler.

Çıktılar `dist/Shaper-<sürüm>-macOS-distribution/` altındadır:

- `Shaper-<sürüm>-macOS-Universal.tar.gz`: Standalone, VST3, AU, presetler ve lisanslar.
- `Shaper-<sürüm>-macOS-Source.zip`: aynı sürümün kaynakları, bağımlılıklar ve build scriptleri.
- `RELEASE-MANIFEST.json`: dosya sağlama değerleri ve derleme bilgileri.

İkili arşiv çalıştırılabilir dosya izinlerini ve bundle bağlantılarını korur.
Paketleme `lipo` ile üç hedefte iki mimariyi, `codesign` ile yerel imzaları ve
arşiv içindeki dosyaların SHA-256 değerlerini denetler. Dört test yalnızca
derleme makinesinin mimarisinde çalışır; diğer mimaride ve Logic/Ableton gibi
DAW'larda ayrıca çalışma testi gerekir.

## GitHub Actions

Music deposundaki kaynak konumu `Shaper/` olmalıdır. Workflow dosyası depo
kökündeki `.github/workflows/shaper-release.yml` konumuna yerleştirilir.
Bu projenin `.github/workflows/macos.yml` dosyası Shaper tek başına bir depo
olduğunda doğrudan çalışır; Music için kopyasında çalışma dizini `Shaper` yapılır.
Actions altında `Shaper Windows and macOS Release` akışını elle başlatın.
Başarılı çalışmanın `Shaper-macOS-Universal` artifact'ı üç çıktı dosyasını içerir.

## İmza ve dağıtım

Paketler ad-hoc imzalanır. Bu, Apple Developer ID imzası veya noter onayı
(notarization) değildir; indirilen dosyalarda macOS güvenlik uyarıları görülebilir.
Genel kullanıma yönelik Developer ID imzalı/noter onaylı dağıtım için Apple
Developer hesabı ve sertifikası gerekir. Bunlar olmadan noter onayı verilmiş
gibi sunulmaz. Kaynak ve ikili arşivleri birlikte yayımlayın; mevcut GPL/AGPL,
MIT ve font lisansları Mac dağıtımında da korunur.
