# F-Saturator: geliştirme ve hata müdahale rehberi

**Fazıl Irak · Düzenleme: 22.10.2026 · Kaynak sürümü: 0.2.0**

Bu kaynak çalışma kopyasıdır; yayımlanan 0.2.0 ZIP’i ve GitHub Release kendiliğinden değiştirilmez. Türkçe not ve biçim düzenlemesinde DSP katsayıları, parametre kimlikleri ve C++ token dizisi korunmuştur. CMake/paketleme yolları yeni klasör düzenine uyarlandı. Üçüncü taraf JUCE/font kaynaklarına geliştirici adınız eklenmedi; özgün lisanslar korunur.

## Başlamak ve sinyal yolunu izlemek

Visual Studio C++ masaüstü araçları ve CMake/Ninja kurulu olmalıdır. Proje klasörünü Visual Studio’nun **Open Folder** seçeneğiyle açabilirsiniz. `scripts/build-windows.cmd` x64 MSVC ortamını kurar, yapılandırır, derler ve CTest’i çalıştırır. VS başka yerdeyse betikteki VSROOT’u değiştirin. CMake projesinde JUCE varsayılanı `../_Dependencies/JUCE`; kayıtlı commit `Research/dependencies.json` içindedir. Tüm Music klasörü taşınırsa bu göreli yapı korunur. Farklı JUCE için `-DIRAK_JUCE_PATH=C:/.../JUCE` verin. Eski build klasörünü başka yola kopyalamayın; CMake önbelleği mutlak kaynak yolları içerir.

Host → Entry `createPluginFilter` → Processor → `prepareToPlay` (tamponlar/gecikme) → `processBlock` (Settings okuma) → DSP `process` (alt bloklar) → `render` (işlem zinciri). GUI ayrı mesaj iş parçacığında Editor/ConcentricKnob ile APVTS’ye bağlıdır; pencere kapalıyken DSP çalışır.

## Hangi hata için nereye bakmalı?

| Belirti | İlk izlenecek alan | Kontrol |
|---|---|---|
| Hiç ses yok | Processor bus, prepareToPlay, processBlock | Mono/stereo eşleşmesi, tampon kanalları, kapasite ve Output/Mix/Bypass |
| Klik/çatlama | render target/weights, reset | Rampalar örnek başına mı? Yeni modele geçişte hafıza temiz mi? |
| Mix’te tarak/faz hissi | prepare latency, wetFractional, dryDelay | DAW bildirimi, kesirli gecikme ve dry/bypass darbe testi birlikte |
| Metalik alias | ADAA process, oversampling, internalRate | Ham/8× karşılaştırma, dx yakın-sıfır yolu, filtrelerin doğru hızdaki katsayıları |
| Tape/Transformer geçmişe bağlı ses | Magnet field/magnetisation, Model record/playback/bass | Kanal/model bağımsızlığı ve reset akışı |
| Drive ile yüksek seviye | totalDrive, compensationDb, autoLevel | Heat +20 dB dahil mi? Referans Auto Gain açık mı? Çıkış limiter değildir. |
| NaN/Inf veya DC | giriş koruması, model denklemleri, dcOutput | Payda sınırları, sonlu sayı denetimi, sessiz kuyruk |
| Kontrol yanlış halkayı değiştiriyor | ConcentricKnob partAt, ids, attachments | İç/orta/dış sıra; geometri ve üç parametrenin ayrı Attachment’ı |
| Otomasyon görünmüyor | begin/end gesture, setValueNotifyingHost | Normalized 0..1 ile fiziksel dB/Hz dönüşümü |
| Preset geri gelmiyor | get/setStateInformation | Binary XML, IRAK_STATE, program 0..15, eski kimlikler |
| Dosya kaydı başarısız ama mesaj yok | choosePreset callback | createDirectory/replaceWithData dönüşü; mevcut sınırlı hata bildirimi |
| Arayüz kapanışında çökme | Editor yıkıcı/chooser callback | SafePointer, timer durdurma, açık gesture ve LookAndFeel ömrü |

## Visual Studio’da adım adım

1. Sorunu bir preset, örnekleme hızı, blok boyu, mono/stereo ve parametre hareketiyle tekrarlanabilir hale getirin. Orijinal DAW projesinin kopyasını kullanın.
2. Standalone ile başlayın; host’a özgüyse sonra VST3’ü DAW içinde açın ve Debug > Attach to Process ile test DAW sürecine bağlanın. Debug derlemesinin sembollerini kullanın. F9 kesme noktası; F10 adım; F11 çağrıya giriş; Call Stack/Locals/Watch ile akışı izleyin.
3. Kontrol sorunu için UI callback → attachment → state → settings; ses sorunu için prepare → render → model → dry/wet yolunu izleyin. Audio thread’de uzun süre kesme noktasında durmak dropout yaratır; bu tek başına DSP hatası kanıtı değildir.
4. Gerçek zamanlı callback’e dosya logu, mesaj kutusu, dosya yükleme, internet veya bekleyen kilit eklemeyin. Sayısal hatayı çevrimdışı render testiyle yakalamak daha kolaydır. Meter atomikleri yalnız GUI telemetrisi içindir.
5. Küçük bir düzeltmeden sonra CTest’i çalıştırın. Davranış değişirse yeni sürüm numarası ve kayıt/preset uyumluluğu incelemesi gerekir; sadece test eşiğini yükselterek hatayı gizlemeyin.

## Test dönüş kodları: reçete gibi yorumlama

`ctest --test-dir build --output-on-failure` çalıştırın. Ayrıntı `build/Testing/Temporary/LastTest.log` içindedir. Test çalışırken CWD build olmalıdır; doğrudan başka klasörden exe başlatırsanız çıktılar ../dist altında farklı yere gidebilir.

| Kod | Kontrol / beklenti | İlk araştırma |
|---|---|---|
| 0 | Normal testler geçti | Tüm DAW’larda garanti değildir; host denemesi ayrıca yapılır. |
| 1 | Alias artığı ham motordan ≥12 dB düşük ve ≤−40 dB olmalı | 8× FIR, ADAA yakın-sıfır yolu ve ölçüm binleri |
| 2 | Dry/bypass darbesi tam bildirilen latency örneğinde | Oversampler+ADAA gecikmesi, fractional ve dairesel tampon |
| 3 | Program state geri çağırma doğru indekste | XML program alanı ve 0..15 sınırı |
| 4 | Preset değişimleri/sessizlikte tüm örnekler sonlu | Sayısal sınır, rampalar, kanal/model hafızası |
| 5 | Sessiz kuyruk büyüklüğü ≤1e−5 | DC temizliği, envelope ve model hafızası |
| 6 | Pentode H2 ≤−65 dBc | Bias ve simetri; test Dynamics/Drive bağlamını koruyun. |
| 7 | Triode H2 ≥−50 dBc | Asimetrik kat ve bias/grid yükü |
| 8 | Referans sinüste Auto Gain sapması ≤0,65 dB | compensationDb tablosu; --calibrate çıktısı; LUFS testi değildir. |
| 9 | 127/256 blok sonucu farkı ≤1e−6 | Smoother ilerlemesi ve kapasite alt blokları |
| 10 | Testte istisna | Terminaldeki asıl I/O/kararsız çıkış mesajını okuyun. |
| 11/12 | Preset yazma/okuma ve byte eşitliği | dist izinleri, dosya yolu ve binary state |
| 13/14/15 | Halka bileşeni, hit-test ve yalnız hedef değişimi | ComponentID, partAt, ids/attachment sırası |
| 16 | Heat/Auto Gain/Bypass varlığı ve dikey sıra | resized ve kalıcı ComponentID sözleşmeleri |

Ölçüm genlik/dBc/artık enerji oranları birbirinin yerine geçmez. Alias testi 8192 örnekte koherent sentetik giriş kullanır; rastgele müzik FFT’siyle aynı eşik kıyası yapılamaz. 44,1/48/96/192 kHz aşırı Drive/Heat testleri sayısal kararlılığı araştırır. Büyük blok ve dry/bypass kontrolleri gerçek zaman planlama garantisi değildir.

## Parametre ve preset değişikliği

Parametre eklemek: layout kimliği/aralık/varsayılan → Settings alanı → settings() atomik okuma → DSP rampası/hedef kullanımı → UI attachment → state/preset ve test. Mevcut `model`, `drive`, `auto_gain`, IRAK_STATE, FIrk, Irak ve bundle ID eski oturum sözleşmesidir. Preset sayısı 16; getNumPrograms, ad/değer dizileri, 0..15 sınırı, %16 gezinme ve paket betiği birlikte değişir.

Giriş/çıkış dB, kesimler Hz; Tone/Bias −1..1, Mix/Dynamics 0..1, Width 0..2. GUI frekans halkası logaritmik; DAW normalizasyonu layout skew’udur. Değer fiziksel mi normalize mi olduğunu her geçişte kontrol edin.

## Yayın ve sahiplik

Paket üretimi için önce başarılı build/test, sonra `scripts/package.ps1`. Bu betik dist ZIP’lerini aynı sürüm adıyla yenileyebilir; önceden yayımlanan dosyayı sessizce değiştirmeyin. Kaynak+Windows dağıtımı lisanslarıyla birlikte eşleşmelidir. Bu çalışma yalnız yerel taşıma/bakım içindir; web veya GitHub’a otomatik push/yayın yapmaz. `GELISTIRME-TR.md` ayrıntılı mevcut geliştirme tarifidir; `docs/KAYNAK_INCELEME_2026-10-22.md` son doğrulama kaydıdır.
