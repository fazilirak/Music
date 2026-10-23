<!-- Türkçe bakım: Fazıl Irak | 22.10.2026 -->
# Shaper 0.8.2 — FAZIL IRAK

[![Build](https://github.com/fazilirak/Music/actions/workflows/shaper-release.yml/badge.svg)](https://github.com/fazilirak/Music/actions/workflows/shaper-release.yml)

[Windows x64](https://github.com/fazilirak/Music/releases/download/Shaper-v0.8.2/Shaper-0.8.2-Windows-x64.zip) · [macOS Universal](https://github.com/fazilirak/Music/releases/download/Shaper-v0.8.2/Shaper-0.8.2-macOS-Universal.tar.gz) · [Releases](https://github.com/fazilirak/Music/releases/tag/Shaper-v0.8.2) · [Source](https://github.com/fazilirak/Music/tree/main/Shaper)

![Shaper](doc/shaper.png)

JUCE tabanlı VST3 ve standalone efekt rack'i: **Gate, Time, Filter, Reverb, Volume, Pan, Saturation, Pitch**. Şirket adı: **FAZIL IRAK**. Pan ve Volume geliştiricisi: **Fazıl Irak**, temel motor GATE-12 / Tilr.

## Dağıtım ve Hakkında (0.8.2)

Saturation ve Pitch, standart Hakkında düzenini kullanır. Lisans/telif bildirimleri ortak alt alandadır. Yeniden dağıtım izni doğrulanmamış varsayılan Reverb IR’si özgün sentetik IR ile değiştirilmiştir; bu nedenle varsayılan Reverb tınısı değişir. Parametre kimlikleri ve preset formatları korunur.

Kaynak ve Windows paketlerini birlikte üretmek için derleme/test sonrası `python scripts/package_distribution.py` çalıştırın. İki ZIP’i aynı indirme yerinde birlikte yayımlayın.

## Yeni presetler ve ipuçları (0.8.1)

- Rack ve iç efektin birlikte açtığı çift ipucu giderildi; tek rack ipucu kullanılır.
- Saturation ve Pitch için 16'şar özgün Motion preset eklendi. Her efektte toplam 40 program vardır; ilk 24 programın sırası korunur.
- Önceki 128 rack'e 16 Saturation Motion ve 16 Pitch Motion eklendi: toplam 160 rack preset.
- 32 yeni efekt preset dosyası `EffectPresets/Saturation` ve `EffectPresets/Pitch` klasöründedir. İlgili efektte Load effect ile açılır; ayrıca efektin dahili preset listesindedir.
- Saturation ve Pitch Motion presetleri ritmik drive, vibrato, perde geçişleri, arpej ve formant hareketleri için özgün eğriler ve Shaper’ın ses motorlarını kullanır.

## Pitch ve Saturation eğrileri (0.8.0)

İki efekt de Gate ile aynı çizim editörünü, 12 pattern'i, Paint / Seq modlarını, Sync / Free / MIDI / Audio tetiklemesini ve Copy / Paste / Invert / Reverse / Double / Clear araçlarını kullanır. Statik saturation transfer grafiği ve büyük Pitch tuşları kaldırılmıştır. Yeni knoblar diğer altı efektle aynı ortak çizim kodunu, 68 x 64 ölçüsünü, beyaz işaretçiyi ve ince renkli yayı kullanır. F Saturator ayrı projedir; Shaper onun arayüzünü, fontlarını veya proje dizinini kullanmaz.

**Saturation:** Üstteki standart efekt ayarları Drive, Tone, Low Cut, High Cut, Bias, Dynamics ve Depth'tir. Model kutusunda Console / Transformer / Tape / Triode / Pentode seçilir. Eğrinin üstü Drive (+ Heat) miktarı, altı 0 dB drive'dır; Depth çizimin katkısını ayarlar. Heat, Auto Gain ve Bypass ayarları korunur. Audio tetikleme seçildiğinde dişli düğmesi tetikleme ayarlarını gösterir; kapatınca efekt ayarlarına dönülür.

**Pitch:** Pitch ve Formant sekmeleri bağımsız çizimler, pattern'ler ve tetikleme ayarları saklar. Pitch eğrisinde orta çizgi sıfır değişim, üst +24 ve alt -24 yarım sestir; Formant eğrisinde uçlar +/-12 yarım sestir. Range her iki eğrinin derinliğini ayarlar. Pitch ve Fine sabit perdeyi, Formant sabit formant ofsetini belirler; çizim bu değerlere eklenir ve toplam perde +/-24, formant +/-12 aralığıyla sınırlanır. Voice base ve Preserve formants ayarları ses karakterini düzenler. Süre/tempo korunur. Ses motoru Signalsmith Stretch'tir; otomatik gam/nota düzeltmesi yapmaz.

İki efektte de ilk 16 ses preseti korunur. Bunlara Rise, Fall, Vibrato, Alternating, Staircase, Drop, Up Sweep ve Down Sweep için 8 çizim preseti eklenmiştir (ilk 24 preset); 0.8.1 sürümündeki 16 Motion presetiyle toplam 40 preset vardır. Her multiband bölgesinin çizimi ve ayarları bağımsızdır. Eski statik Pitch/Saturation durumları yüklenirken Depth/Range sıfırlanır; kayıtlı ses ayarları korunur. Eğri ve ses ayarları efekt/rack presetleri ve DAW oturumuyla birlikte saklanır.

Ses ve iki çizim, MIDI/audio tetikleme kaynakları arasında hizalı kalması için sabit 15 ms kontrol tamponu kullanır. Toplam gecikme Saturation'da 48 kHz için 786 örnek (16.375 ms), Pitch'te 6480 örnek (135 ms) olur; DAW'a bildirilir. Dry/wet ve bypass bu gecikmeyle hizalanır. Spektral pitch ve formant işlemleri, büyük transpozisyonlarda ve vurmalı kaynaklarda tını/transient değişimleri oluşturabilir.

Pitch motoru Signalsmith Stretch kullanır. Bağımlılık sürümleri ve MIT bildirimleri Pitch-dependencies.json / THIRD-PARTY-NOTICES.txt içindedir.
## Kullanım

- Rack boş açılır. Üst paletin **+** butonundan efekt ekleyin. Her efekt türü bir kez eklenebilir.
- Efekt kartına tıklayarak editörünü açın. Kartları yatay olarak **sürükleyip bırakarak** sıralayın; ses soldan sağa bu sırayla işlenir.
- Kartın sağ üst köşesindeki **×** efekti zincirden çıkarır. Kartın altındaki **ON / BYPASSED** alanı efekti geçici olarak bypass eder. Kaldırılan efektin ayarları rack içinde korunur; yeniden eklendiğinde geri gelir.
- Renkler: Gate kiremit, Time yeşil, Filter mavi, Reverb mor, Pan kırmızı, Volume sarı, Saturation F EQ turuncusu, Pitch turkuaz.
- Paletin üstündeki **RACK PRESET** alanı bütün zincir içindir. Disk simgesi kaydeder, klasör simgesi dosya yükler; oklar önceki / sonraki presete geçer. Palet, sıralama, bypass durumları ve sekiz efektin tüm eğri/parametreleri birlikte saklanır. Varsayılan klasör Belgeler / Shaper Presets'tir; dosya penceresinde başka bir klasör de seçilebilir.
- Efekt preset alanı ve düzenleme araçları **efektin altında**, tek kompakt simge çubuğundadır. Düğmelerin üzerine gelince açıklamaları görünür. Alt combobox seçili efektin hazır presetlerini açar; yanındaki oklar önceki / sonraki preseti seçer.
- Alt çubuktaki disk / klasör simgeleri, seçili efektin tüm bantlarını `.shapereffect` dosyası olarak kaydeder ve yükler. Farklı efekt türüne ait dosyalar reddedilir. Multiband açıkken yerleşik efekt preset listesi seçili bandın presetini değiştirir.
- Rack preset alanında **önceki / sonraki** düğmeleri ve 10 grupta **160 özgün yerleşik preset** bulunur: Ducking, Rhythm, Filter Motion, Stereo, Time Motion, Space, Multiband , Creative Racks, Saturation Motion ve Pitch Motion. Palet, efekt sırası ve tüm bant ayarları birlikte yüklenir.
- Alt çubuktaki simgeler **Options, Copy, Paste, Invert, Reverse, Double ve Clear** işlemlerini yapar. Sequencer açıkken Clear yerine Reset kullanılır; uygun olmayan işlemler devre dışı kalır. Komutlar seçili efektin aktif düzenleme alanına uygulanır.
- Eski sağ menü bütün efektlerden kaldırıldı. UI Scale ve Load bu menü çubuğunda yer almaz. Sağ üstteki **About** butonu sekiz efektin kendi About sayfalarını sekmeler halinde açar. Orijinal geliştirici bilgileri korunur; Pan ve Volume'de Fazıl Irak adı gösterilir.
- Genel multiband / crossover yoktur. Sekiz efektin tamamında, **Time dahil, frekans grafiği solda, kompakt efekt ayarları sağda** yer alır; eğri editörü altta tam genişliği kullanır. Bands grafiğini açmak/kapatmak ses ayarını değiştirmez. Sol sınır **20 Hz**, sağ sınır **20 kHz** iken yalnızca **Mid bütün sesi işler**; crossover ses yoluna girmez. Sol kenarı içeri sürükleyerek Low, sağ kenarı içeri sürükleyerek High ekleyin. Yalnızca bir kenarı değiştirirseniz iki bant, iki kenarı değiştirirseniz üç bant çalışır. Grafikte bir bölgeye tıklayarak veya Low/Mid/High düğmeleriyle düzenlenecek bandı seçin. Her bandın eğrisi ve efekt ayarları bağımsızdır. Kenara çift tıklama o bandı kaldırır; **Full range** iki kenarı sıfırlar. Grafik sesin giriş spektrumunu da gösterir.
- Efekt isimleri yalnızca rack paletinde gösterilir. **Tempo / Trigger / Pat. Sync** aynı satırdadır; 1–12 pattern düğmeleri hemen altındadır. Sağ panelde yazılar, düğmeler ve knoblar aynı yatay/dikey oranda ölçeklenir; şekilleri dikey uzamaz.
- Sekiz efektin sağ üstünde aynı **Stereo / çıkış seviye göstergesi / dB / Mix** paneli bulunur. Mix yüzdesi daha büyük ve efektin kendi rengindedir. Değerleri sürükleyerek veya fare tekerleğiyle değiştirin; çift tıklama varsayılan değeri geri getirir. Çıkış seviyesi seçili banda uygulanır ve presetle birlikte saklanır. Stereo alanı Gate, Filter, Pan ve Volume'de mevcut sol/sağ modülasyon fazını; Time, Reverb, Saturation ve Pitch'da stereo genişliğini düzenler. Üzerine gelince sayısal değer görünür.
- Ortak kontroller **MODULATION**, efekte özgü kontroller **EFFECT SETTINGS** bölümünde toplanır. Ortak düğmelerin, pattern satırının ve eğri araçlarının boyutları ve konumları bütün efektlerde aynıdır. Time'ın Phase / Smooth / Attack / Release / Tension kontrolleri de bu ortak düzende yer alır; mevcut motor parametrelerine bağlıdır. Ek EQ/Decay ve tetikleme görünümleri kendi bölümünde açılır.
- **Snap** işlevi korunur; açık/kapalı durumu dolgulu zemin yerine çerçeve vurgusuyla gösterilir.
- **MIX / OUTPUT** master dry/wet ve çıkış seviyesidir. Bypass ve dry sinyal efekt gecikmeleriyle hizalanır.
- **EFFECT PRESET**, bütün rack'ten bağımsız olarak yalnızca seçili efektin hazır presetini değiştirir.

## Pan

Gate-12 ile aynı çok noktalı eğri editörü, 12 pattern, paint modu, sequencer, MIDI / audio tetikleme, tempo senkronizasyonu, Hz modu, stereo faz farkı, attack/release, tension ve dahili frekans seçimi özelliklerini kullanır. Volume yerine **equal-power stereo balance** uygular: 0 sol, 0.5 merkez, 1 sağdır. Merkezde her iki kanalın seviyesi değişmez; iki kanalda aynı sinyal varken uçlarda açık kanal yaklaşık +3 dB olur ve toplam güç korunur.

Pan'a özel **20 preset**: Centre, Hard Left/Right, iki yönlü Sweep, Sine/Triangle Auto Pan, Smooth Wide, Narrow Stereo, Ping Pong 2/4/8, Soft Bounce, Left/Right Accent, Centre to Wide, Wide to Centre, Triplet, Swing ve Subtle Drift. Sabit presetler az sayıda nokta kullanır; ritmik yön değişimlerinde kısa rampalar ve attack/release yumuşatması bulunur. Hareketli presetler uygun 1 veya 2 bar süresiyle yüklenir. Preset seçimi min/max, phase, stereo offset ve smoothing ayarlarını da belirler.

## Volume

Gate-12'nin volume motorunu sidechain amaçlı kullanır. Gate'in pattern, MIDI/audio trigger, paint, sequencer, tension ve Bands özellikleri korunur. Curve kutusunun sağında tek sırada **8 küçük eğri kutusu** bulunur. Bunlar Fazıl Irak'ın gönderdiği **1.shapereffect–8.shapereffect** dosyalarından, numara sırası korunarak alınmıştır. İlk açılışta Volume 1 yüklenir. Bir kutuya tıklamak seçili bandın kaydedilmiş eğri ve efekt ayarlarını yükler; rack'in bant sınırlarını değiştirmez. Aynı presetler EFFECT PRESET listesindedir.

Önceki 16 yaklaşık eğri kaldırılmıştır. Yeni 8 presetin küçük çizimleri de aynı kayıtlı nokta ve tension verilerinden, Volume'un eğri motoruyla üretilir. Özgün dosyalar paketin `VolumePresets` klasöründe, dosya sağlama değerleri `manifest.json` içinde bulunur. Kaydedilmiş 12 pattern ve efekt parametrelerinin kaynak dosyalarla eşleşmesi otomatik test edilir.

## Time hızlı presetleri

Time sayfasında 8 yeni küçük eğri kutusu vardır: **Stutter 1/8, Stutter 1/16, Scratch Sweep, Scratch Double, Tape Stop Slow, Tape Stop Fast, Pitch -12 st, Pitch +12 st**. Bunlar orijinal 79 presete ek olarak listede de bulunur. Bir kutuya tıklamak eğriyi, tempo senkronizasyonunu, mix ve smoothing ayarlarını birlikte yükler. Time preset dosyaları ve DAW durumu bu seçimi de saklar.

Time hızlı presetleri stutter, scratch ve tape-stop için bağımsız eğriler içerir. Pitch örnekleri Time motorunun okuma hızını 0.5x ve 2x yapar; tempo süresini koruyan ayrı bir pitch-shifter algoritması değildir. Perde oranları 440 Hz sinyal ile otomatik test edilir.

## Rack presetleri

160 preset ayrıca paketin `FactoryPresets` klasöründe, 10 kategoriye ayrılmış `.shaper` dosyaları olarak bulunur. `Load rack` ile herhangi birini açabilirsiniz. Ayrı banka arşivi: `Shaper-160-Rack-Presets.zip`. Dosya isimleri ve efekt sıraları `FactoryPresets/index.json` içinde listelenir. Dahili listeden seçmek için bu dosyaları kurmak gerekmez.

160 rack preseti ritmik ducking, gate, filtre, stereo hareket, zaman ve reverb düzenlerini içerir. Preset dosyaları ve sayısal eğri verileri bağımsız oluşturuldu. Ses kararlılığı otomatik olarak kontrol edilir; müzikal değerlendirme farklı kaynaklarla DAW içinde yapılmalıdır.

## Derleme ve çıktılar

Windows x64: Visual Studio 2026 C++ masaüstü araçları, Windows SDK, CMake, Ninja ve Python 3 gerekir. `scripts/build-windows.cmd` bu bilgisayardaki Visual Studio ve Python yollarıyla yapılandırılmıştır.

```bat
scripts\build-windows.cmd
```

Script ses ve arayüz entegrasyon testlerini çalıştırır. `scripts/package.ps1` çıktıları `dist` klasöründe toplar. Standalone: `dist/Standalone/Shaper.exe`. Plugin: `dist/VST3/Shaper.vst3` paketinin tamamı. VST3 otomatik olarak sistem plugin klasörüne kurulmaz.

Tempo, MIDI, opsiyonel stereo sidechain ve efekt otomasyonu JUCE üzerinden aktarılır. Efekt başına frekans, pattern ve diğer parametreler DAW'a ayrı kimliklerle sunulur. Gerçek DAW ve ses aygıtı üzerinde manuel uyumluluk testi ayrıca yapılmalıdır.

0.1.0 rack dosyaları açılırken önceden seçili bandın dört efekti tek zincire alınır. Eski dış crossover kaldırılmıştır. 0.2.0 palet sıralamaları altıncı efektin eklenmesine göre dönüştürülür. Tek motor durumları, Time dahil, Mid / Full motoruna yüklenir; ses tam aralıkta kalır. Time'ın yeni Low/Mid/High motorları ve bant sınırları efekt/rack presetlerinde birlikte saklanır.

## Kaynak yapısı

- `Source`: Tek zincir ses motoru, efekt paleti, sürükle-bırak, komut çubuğu, presetler ve parametre köprüleri.
- `upstream`: Tiagolr'ın orijinal dört deposu. Sürümler `upstream-lock.json` içinde kayıtlıdır.
- `scripts/prepare_sources.py` ve `scripts/module_extensions.py`: Namespace uyarlamaları, menü köprüsü, Gate tabanlı Pan ve Volume kaynaklarını üretir. Orijinal depolar değiştirilmez. Türevler `build/modules/pan12` ve `build/modules/volumef` altındadır.
- `Source/MultibandProcessor.*`: Efekt başına üç bağımsız motor, crossover, bant editörü ve durum saklama.
- `Source/FactoryPresets.cpp`: 160 yerleşik rack presetinin isimleri, özgün eğrileri, efekt sıraları ve ayarları.
- `Source/Embedded/PanPresets.h`: Pan presetlerinin matematiksel tanımları. `SidechainPresets.h`, `VolumePresetData.h` ve `VolumePresets`: Fazıl Irak'ın sekiz kayıtlı Volume preseti ve küçük önizlemeleri.
- `Source/BandGraph.h`: Sürüklenebilir frekans grafiği, bant seçimi ve giriş spektrumu.
- `Tests`: Boş rack bypass'ı, palet işlemleri, ses sırası, Pan L/C/R, sekiz efektin orantılı başlık / Snap arayüzü, Time dahil bağımsız Low/Mid/High ses izolasyonu, efekt preset dosyaları, 160 benzersiz rack durumu ve preset gezinmesi, Volume ducking / mini kutular, About, otomasyon ve eski preset dönüşümleri.

## Kaynak projeler

Orijinal geliştirici: **tiagolr / tilr**.

- [GATE-12](https://github.com/tiagolr/gate12)
- [TIME-12](https://github.com/tiagolr/time12)
- [FILT-R](https://github.com/tiagolr/filtr)
- [REEV-R](https://github.com/tiagolr/reevr)

Gate12, Time12 ve Reevr GPLv3; Filtr AGPLv3 lisanslıdır. JUCE 8 AGPLv3 / ticari çift lisanslıdır. Dağıtımda GPL/AGPL koşulları ve bağımlılık bildirimleri birlikte uygulanmalıdır. 0.8.2 dağıtımı, aynı sürümün tam kaynak ZIP’i ve lisans bildirimleriyle birlikte hazırlanır. Yeniden dağıtım izni doğrulanmamış Samplicity IR pakete alınmaz; varsayılan IR, Shaper için üretilen GPLv3 lisanslı dört kanallı sentetik kaynaktır. Ayrıntılar: [yayın ve lisanslar](docs/YAYIN_LISANSLARI_TR.md).

## Saturation / F Saturator

Shaper Saturation motoru `Source/Saturation/` içinde bağımsız bir kaynak sürümü olarak tutulur; ayrı F Saturator projesine derleme veya arayüz bağımlılığı yoktur. Aynı motor Shaper içinde yedinci efekt **Saturation** olarak bulunur. Console, Transformer, Tape, Triode ve Pentode karakterleri; Drive/Heat, Tone, low/high cut, Bias, Dynamics, Auto Gain ve 16 özgün preset içerir. 8x FIR oversampling ve ADAA kullanılır. Her bant bağımsızdır; preset dosyaları bütün bantları saklar.

Saturation ve Pitch, Gate ile aynı tempo/pattern eğri araçlarını içerir. Copy/Reverse işlemleri seçili çizime uygulanır. Ortak sağ üst Mix/Stereo/dB paneli aynı konumdadır. Eski altı efektin 720 sıralama/otomasyon indeksi korunur; Saturation için ayrı zincir konumu parametresi kullanılır. Eski rack dosyaları Saturation kapalı olarak açılır.

Algoritma araştırması ve sınırlar: `Source/Saturation/DSP-RESEARCH.md`. Motor özgün hibrit tasarımdır; belirli bir cihaz veya Decapitator ile ölçülmüş ses eşdeğerliği iddia edilmez.


## Yeni geliştirme konumu

`Codex/Music/Shaper` içinden çalışın. [Türkçe geliştirici ve hata müdahale rehberi](docs/GELISTIRICI_REHBERI_TR.md). Eski mutlak build yollarını yeniden kullanmayın.

## macOS derlemesi

Apple Silicon + Intel için Standalone, VST3 ve AU derleme akışı: [Mac rehberi](docs/MACOS_TR.md). Kaynaklar macOS üzerinde `bash scripts/build-macos.sh` ile derlenir. Gerçek Mac çıktıları GitHub Actions derlemesi başarıyla tamamlandığında oluşturulur.
