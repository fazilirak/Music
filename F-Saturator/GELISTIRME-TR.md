<!-- Fazıl Irak | Son düzenleme: 24.10.2026 -->
# F Saturator — Türkçe geliştirme rehberi

Bu rehber 0.4.0 kaynakları içindir. Türkçe yorumlar kodun akışını açıklar. Yeni Character / Harmonic Balance / Analog Drift grubu için docs/CHARACTER_TR.md belgesine bakın; üç yeni kontrol sıfırken eski ses korunur. İlk denemelerinizde özgün preset ve DAW projelerinizin kopyasını kullanın.

## Nerede değişiklik yapmalıyım?

| Yapılacak değişiklik | Dosya / bölüm |
|---|---|
| Renk, font, düğme görünümü | `Source/PluginEditor.cpp`, `Editor::Theme` |
| Pencere boyutu ve kontrol yerleşimi | `PluginEditor.cpp`, `Editor::resized`, `paint`, kurucu |
| Halka şekli, hassasiyet, sayı girişi | `Source/ConcentricKnob.h` |
| Parametre sınırı veya varsayılanı | `PluginProcessor.cpp`, `Processor::layout` ve `SaturationDSP.h`, `Settings` |
| Fabrika presetinin adı veya ayarları | `PluginProcessor.cpp`, `getProgramName` ve `setCurrentProgram` |
| Doygunluk modelinin sesi | `SaturationDSP.cpp`, `Model::process`, `ADAA`, `Magnet` |
| Auto Gain telafisi | `SaturationDSP.cpp`, `compensationDb`; test uygulamasının `--calibrate` çıktısı |
| Filtre, Tone, Mix, Stereo, Bypass | `SaturationDSP.cpp`, `render` |
| Kaydetme / yükleme biçimi | `PluginProcessor.cpp`, `getStateInformation`, `setStateInformation` |
| Preset dosyası penceresi | `PluginEditor.cpp`, `choosePreset` |
| Ürün sürümü ve derleme hedefi | `CMakeLists.txt`, `scripts/package.ps1`, dağıtım belgeleri |
| Bir değişikliğin doğrulanması | `Tests/DSPTests.cpp` |

## Çalışma akışı

JUCE `Entry.cpp` üzerinden bir `Processor` oluşturur. Processor, DAW parametrelerini `AudioProcessorValueTreeState` (APVTS) içinde tutar. `prepareToPlay` motoru hazırlar ve gecikmesini DAW'a bildirir. `processBlock`, parametreleri `Settings` yapısına okuyarak DSP'ye verir. `Editor`, aynı parametrelere Attachment nesneleriyle bağlanır; panel kapalıyken de ses işlemeye devam edilir.

Ses akışı:

```text
Giriş → 8× FIR oversampling → Low Cut → Drive + Heat
      → seçili karakter / iki ADAA katı / dinamik ve manyetik durum
      → Tone → High Cut → DC temizleme → host hızına dönüş
      → kesirli gecikme düzeltme → Auto Gain → wet stereo genişliği
      → gecikmesi eşlenmiş dry/wet Mix → Output → hizalanmış Bypass
```

ADAA, antitürev farklarıyla doğrusal olmayan işlemin aliasing üretimini azaltan yöntemdir. Manyetik kol geçmiş örneklere bağlıdır; yalnız statik bir eğriden oluşmaz. Ekrandaki transfer grafiği bu dinamik davranışın tam ölçümü değildir.

## Parametre ekleme veya değiştirme

1. `Processor::layout` içinde kimlik, görünen ad, aralık ve varsayılanı tanımlayın.
2. `Settings` yapısını ve `Processor::settings` okumasını birlikte güncelleyin.
3. Gerekliyse DSP hazırlığına ve `render` içindeki yumuşatma/hedef akışına ekleyin.
4. Editörde kontrolü doğru parametreye Attachment ile bağlayın.
5. Preset, kayıt/geri yükleme ve blok boyu davranışını sınayın.

`model`, `drive`, `auto_gain` gibi mevcut kimlikleri değiştirmeyin. `IRAK_STATE`, `FIrk`, `Irak` ve `com.fazilirak.irak` eski kayıtlarla ilişkilidir. Görünen ürün adını değiştirmek için bunları yeniden adlandırmanız gerekmez. Durum biçimini değiştirirseniz eski XML için açık bir geçiş planlayın; yalnız `version` alanını artırmak dönüşümü otomatik yapmaz.

`ConcentricKnob` içindeki frekans ölçeği görseldir. DAW normalizasyonunu `layout` belirler. Birini değiştirirken diğerini dikkate alın; eski otomasyon eğrilerini kazara farklı fiziksel değerlere eşlemeyin.

## Preset düzenleme

İsimler ile `Preset` değer satırları aynı sıradadır. Model indeksleri 0=Console, 1=Transformer, 2=Tape, 3=Triode, 4=Pentode. Her satırda Drive, Tone, Low Cut, High Cut, Bias, Dynamics, Mix ve Heat bulunur. Fabrika preset çağrısı Auto Gain'i açar; Output=0, Width=1 ve Bypass=kapalı yapar.

Preset sayısını artırmak yalnız bir satır eklemek değildir: `getNumPrograms`, 0..15 sınırları, `%16` gezinme kodu, testlerdeki beklentiler ve paket betiğinin 16 dosya kontrolü birlikte güncellenmelidir. Mevcut `.irakpreset` dosyaları binary XML durumudur; metin düzenleyiciyle rastgele değiştirmeyin.

## Gerçek zamanlı işlem kuralları

- Ses callback'ine dosya erişimi, internet isteği, pencere açma veya bekleyen kilit eklemeyin.
- Tampon kapasitesini `prepare` aşamasında ayırın; büyük host blokları mevcut alt blok yolunu kullanır.
- Parametre rampalarını kanal başına değil, ilgili örnekleme hızında örnek başına ilerletin.
- Kuru/işlenmiş yolların gecikmesini birlikte koruyun. 8× oranı, ADAA veya filtre türü değişirse gecikme testlerini yeniden çalıştırın.
- Manyetik ve filtre hafızasını kanallar arasında paylaşmayın. Ortak Dynamics zarfı mevcut tasarımın bilinçli bir parçasıdır.
- GUI için atomik tepe göstergelerini kullanın; ses motorunu çizim kodundan kilitlemeyin.

## Derleme

Gerekli araçlar: Windows x64, C++ masaüstü araçları bulunan Visual Studio, CMake 3.24+ ve Ninja. Yerel `scripts/build-windows.cmd` Visual Studio 2026 Community yolunu kullanır; başka kurulumda `VSROOT` satırını uyarlayın.

JUCE, GitHub kaynağına kopyalanmaz. Kullanılan sürüm `Research/dependencies.json` içinde tam commit ile kayıtlıdır. JUCE'yi ayrı bir klasöre alın; aşağıdaki komutlar Visual Studio x64 Native Tools terminali içindir:

```powershell
git clone https://github.com/juce-framework/JUCE.git ../JUCE
git -C ../JUCE checkout 10a589619b452c261b2940767eb253171eb5a823
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIRAK_JUCE_PATH=../JUCE
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Çıktılar `build/IRAK_artefacts/Release/Standalone` ve `.../VST3` içindedir. Başarılı testler `dist` içinde 16 preset, iki ekran görüntüsü, altı orijinal WAV ve DSP raporu üretir. Testlerin GUI başlatması, bileşen çizimi içindir.

Auto Gain katsayıları gerçekten değiştirildiyse:

```powershell
Push-Location build
.\IRAKTests_artefacts\Release\IRAKTests.exe --calibrate
Pop-Location
```

Kalibrasyon çıktısı tabloya elle aktarılır. Her güncellemeden sonra normal testleri yeniden çalıştırın. Bu referans sinüs telafisi LUFS normalizasyonu değildir.

## Testlerin kapsamı

Mevcut testler alias artık enerjisi, harmonik karakter, referans Auto Gain, farklı örnekleme hızları, blok boyu tutarlılığı, büyük bloklar, dry/bypass gecikmesi, preset dosyaları, halka hedefleri, GUI görüntüsü ve sessiz kuyruk kontrollerini içerir. Bunlar bütün DAW'larda kurulum testi, donanım eşdeğerliği veya dinleme değerlendirmesinin yerine geçmez.

## Paket ve GitHub yayını

Testler geçtikten sonra `powershell -ExecutionPolicy Bypass -File scripts/package.ps1` mevcut dağıtım arşivlerini üretir. Betik her çalıştığında aynı sürümün ZIP'lerini yeniler; daha önce yayımlanmış sürümü sessizce değiştirmeyin. Sonraki ses/kod değişikliklerinde yeni sürüm numarası kullanın.

Depoya `Source`, `Tests`, `Resources`, `Research` içindeki belgeler, `scripts`, CMake ve lisans belgeleri alınır. `build` ve `dist` sürüm kontrolüne girmez; Windows ZIP'i GitHub Releases varlığı olur. Yalnız kendi kodunuza açıklama eklenmiştir; üçüncü taraf fontların özgün lisans ve kaynak bildirimleri korunur.

F Saturator kodu AGPLv3-or-later olarak lisanslıdır; JUCE ayrıca kendi lisansına tabidir. Kaynakları ve tam JUCE sürümü/derleme talimatlarını Windows sürümüyle eşleştirin. GitHub herkese açık depo ve etiketli Release bu eşlemeyi görünür kılar. Sitedeki indirme düğmesi Release dosyasına, kaynak bağlantısı depoya yönlenebilir.

Database Doctor ayrı bir depoda ve kendi lisans kararıyla yayımlanmalıdır. Bu projenin AGPL tercihi ona kendiliğinden uygulanmaz. Database Doctor kaynaklarını herkese açmak ile yalnız kurulum dosyasını Releases üzerinden dağıtmak ayrı kararlardır.


Yeni klasör düzeni ve ayrıntılı hata müdahalesi: [Bakım rehberi](docs/HATA_AYIKLAMA_TR.md).
