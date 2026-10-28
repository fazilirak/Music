# F-Kick / F-TransForm — macOS Universal

Geliştirici: **Fazıl Irak**. Özgün uygulama kodunun ve bağımlılıkların telif/lisans bildirimleri dağıtımlarda korunur.

Mac yayın akışı `.github/workflows/f-originals-macos.yml` dosyasındadır. Uygulama sürümlerini, eklenti kimliklerini, DSP davranışını ve oturum/preset biçimlerini değiştirmeden mevcut sürümlere macOS paketleri ekler.

## Derleme

macOS ve Xcode komut satırı araçları gerekir. Depo kökünden:

```sh
python3 -m pip install 'cmake>=3.24,<4' ninja
cd F-Kick  # F-TransForm için bu klasör adını değiştirin
python3 scripts/bootstrap_juce.py
# Yalnızca F-TransForm: python3 scripts/bootstrap_fonts.py
cmake -S . -B build-macos -G Ninja -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-macos --parallel 3
ctest --test-dir build-macos --output-on-failure --timeout 600
python3 ../scripts/package_originals_macos.py .
```

F-Kick için CMake komutuna `-DCMAKE_CXX_FLAGS=-ffp-contract=off` eklenir. Bu ayar Apple Clang'ın çarpma/toplama işlemlerini FMA olarak birleştirmesini kapatır; eski Windows referans seslerinin ayrı işlem yuvarlamasını korumayı amaçlar. Windows byte-byte karşılaştırması ve diğer platformlardaki iki 24-bit PCM basamağı sınırı korunur. İlgili derleyici seçeneği: [Clang kayan nokta davranışı](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior).

VST3, Audio Unit ve bağımsız uygulama oluşturulur. Her Mach-O dosyasının `arm64` ve `x86_64` mimarilerini taşıdığı, paket sürümü ve ad-hoc imzası paketlemeden önce doğrulanır. Başarısız CTest sonucu paketlemeyi durdurur. Gerçek Intel Mac ve üçüncü taraf DAW testleri, yerel macOS CI testlerinden ayrı değerlendirilmelidir.

## Kurulum

macOS 11.0 veya üzeri gerekir. `*-macOS-Universal.tar.gz` arşivini açın:

- `VST3/*.vst3` paketinin tamamını `~/Library/Audio/Plug-Ins/VST3/` klasörüne kopyalayın.
- `AU/*.component` paketinin tamamını `~/Library/Audio/Plug-Ins/Components/` klasörüne kopyalayın.
- Host içinde eklentileri yeniden taratın. Bağımsız uygulama `Standalone/*.app` altındadır.

Paketler ad-hoc imzalıdır; Developer ID imzası ve Apple noter onayı içermez.

## Kaynak ve doğrulama

Her Mac ikilisiyle aynı yayında `*-macOS-Source.zip` bulunur. Bu arşiv uygulamayı, testleri, kaynakları, derleme betiklerini, tam sabitlenmiş JUCE kaynaklarını ve F-TransForm için tam URW font koleksiyonunu içerir. Arşivden bağımsız derleme yönergeleri `BUILD-MACOS.txt` içindedir.

SHA-256 değerleri, derleme commit'i, mimariler, sürümler ve test kapsamı `macOS-RELEASE-MANIFEST.json` dosyasında yer alır. Diğer platformların yayımlanmış dosyaları değiştirilmez.
