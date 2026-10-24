<!-- Türkçe bakım: Fazıl Irak | 22.10.2026 -->
# F EQ 0.1.10

1970'lerin tilt EQ yaklaşımından esinlenen yerel ses efekti. Model F'nin koyu gri paneli, turuncu işaretçileri, tipografisi ve knob tasarımı kullanılır. Rust + NIH-plug + baseview + softbuffer + SVG/resvg: Model F ile aynı teknoloji. Windows x64 için VST3, CLAP ve bağımsız EXE.

Büyük düğmenin iç kapağı Minimoog tarzı üstten görünen, yivli hatlara sahip bir siluettir; Model F grisi (#292f31) ile düz renkte çizilir. Dış frekans halkası Model F turuncusudur (#ed6c46). Her iki kontrolde flat tasarım kullanılır; gradyan, parlama, ışık yansıması veya gölge yoktur.

Arayüz 800 × 418 piksele toplandı: tek response grafiği başlığın sağında, low/high cut büyük düğmenin solunda, bass lift/Q/saturation/volume sağda.

Frekans düğmelerinin görsel hareketi logaritmiktir: minimum ve maksimum değerler ölçeğin uç çizgileriyle eşleşir; eşit açısal hareket eşit frekans oranı verir. Eski oturumların ve otomasyonun host normalizasyonu korunur.

Eski oturumların Tilt Q değeri korunur ve yeni orta halkada gösterilir. Yeni Bass Q eski sabit bass shelf tepkisini korumak için varsayılan 0.707 olarak yüklenir. İki Q kontrolü ayrı ayrı otomasyona açıktır.

## GitHub releases / Windows ve macOS

- [Windows, Mac Universal ve tam eşleşen kaynaklar](https://github.com/fazilirak/Music/releases/tag/F-EQ-v0.1.10)
- [Otomatik Windows/macOS derlemesi](https://github.com/fazilirak/Music/actions/workflows/rust-audio-release.yml)
- [Lisans ve kaynak paketleme açıklaması](docs/RELEASE-LICENSING.md) · [COPYING](COPYING)

macOS: Universal arm64+x86_64 **VST3, CLAP ve bağımsız APP**; macOS 11+. VST3'ü `~/Library/Audio/Plug-Ins/VST3`, CLAP'ı `~/Library/Audio/Plug-Ins/CLAP`, APP'ı Applications'a koyun. AU sürümü yoktur. Paketler ad-hoc imzalıdır; Developer ID imzası ve Apple noterlemesi yoktur. Her iki platform için aynı sürümün kaynak ZIP'i birlikte sunulur.

English: Free GPLv3-covered Rust/NIH-plug edition, with preserved third-party licences. Windows x64 and macOS Universal: VST3, CLAP, standalone. Complete matching source, vendored dependencies and build scripts accompany downloads. Mac builds are ad-hoc signed, not notarised; independent DAW and hardware validation remains necessary.

## Kontroller

Response grafiğinin arkasındaki soluk yeşil/gri alan, gerçek zamanlı çıkış spektrumudur. Turuncu çizgi EQ eğrisini gösterir. Analizör 4096 örnekli Hann penceresi ve FFT kullanır; stereo kanalların enerjileri ayrı ölçülüp birleştirilir, ters fazlı stereo içerik kaybolmaz. Görüntü yaklaşık 30 kez/saniye güncellenir ve ses durunca sönümlenir. FFT yalnız arayüz iş parçacığında çalışır; ses işleme yalnız kilitsiz örnek tamponuna yazar. Analizör dinleme sinyaline gecikme veya ses değişikliği eklemez. docs/analyzer-preview.png örnek sinyalin gerçek FFT'sini gösterir.

| Kontrol | Aralık / davranış |
| --- | --- |
| Tilt | −12…+12 dB. Sağa: tizleri artırır, basları azaltır. Sola: basları artırır, tizleri azaltır. Merkez nötr. |
| Pivot | 200…4000 Hz, varsayılan 900 Hz. Tilt eğrisinin sıfır dB'den geçtiği frekans. |
| Bass Lift | 300 Hz low shelf, −12…+12 dB. |
| Tilt Q (orta halka) | 0.1…2.0, varsayılan 0.707. Tilt shelf geçişinin şeklini değiştirir. |
| Bass Q | 0.1…2.0, varsayılan 0.707. Yalnız 300 Hz Bass Lift shelf geçişini şekillendirir. |
| Low Cut | OFF / 20…500 Hz high-pass; en solda kapalı. Altındaki 12/24/36/48/96 dB/oktav düğmeleri eğimi seçer. |
| High Cut | 1000…22000 Hz / OFF low-pass; en sağda kapalı. Altındaki 12/24/36/48/96 dB/oktav düğmeleri eğimi seçer. |
| Analog | Luftikus'tan uyarlanan çok düşük seviyeli analog gürültü ve bant kazanç toleransı. Açıkken turuncu. |
| Saturation | %0…100, ayrı harmonik doygunluk. %0 tam temizdir. Analog'dan bağımsızdır. |
| Volume | −24…+12 dB çıkış seviyesi. |
| EQ ON | Bypass: EQ, Analog, Saturation ve çıkış kazancını birlikte devreden çıkarır. |

Büyük kontrol iç içe üç bağımsız düğmeden oluşur: inceltilmiş turuncu dış halka Pivot frekansını, kırık beyaz orta halka Tilt Q değerini, gri iç kapak Tilt seviyesini değiştirir. Bass Lift yanındaki Bass Q düğmesi yalnız Bass Lift filtresini etkiler. Fareyle iç kapağı, orta halkayı veya dış halkayı hedefleyip yukarı/aşağı sürükleyin ya da tekerleği kullanın. SHIFT hassas ayar, CTRL/ALT + tıklama hedeflenen düğmeyi varsayılana döndürür. Alt eğim düğmelerine tıklayarak low/high cut keskinliklerini bağımsız seçin. Tüm kontroller DAW otomasyonuna açıktır; ayarlar DAW projesinde saklanır. Grafikte turuncu çizgi toplam doğrusal EQ tepkisidir (Volume, rastgele Analog sapması ve Saturation hariç). OUTPUT ışığı sinyalde yeşil, 0 dBFS ve üzerinde turuncu olur.

F EQ logosuna tıklamak About penceresini açar. Sürüm ve geliştirici bilgileri burada gösterilir. CLOSE, sağ üst çarpı, Esc/Enter veya pencere dışına tıklama ile kapanır. VST3 host klavye iletimi ve yerel klavye olayları Esc/Enter/keypad Enter için desteklenir. About açıkken alttaki ses kontrolleri fareyle değiştirilmez; ses işleme devam eder.

## Kullanım

- VST3: `F EQ.vst3` klasörünü DAW'ın taradığı bir VST3 konumuna kopyalayın. Standart Windows konumu `C:\Program Files\Common Files\VST3` (yönetici izni gerekebilir). İçteki dosyayı tek başına taşımayın.
- CLAP: `F EQ.clap` dosyasını DAW'ın CLAP klasörüne kopyalayın; standart konum `C:\Program Files\Common Files\CLAP`.
- DAW içinde bir ses kanalına insert efekti olarak ekleyin. Mono ve stereo desteklenir; stereo kanallar bağımsız filtre belleği kullanır.
- EXE: `F EQ.exe --help` ses aygıtı seçeneklerini listeler. Giriş aygıtı komut satırında seçilmelidir; seçilmezse ses girişi bağlanmaz. PowerShell'de `& '.\F EQ.exe' --backend wasapi --input-device 'AYGIT ADI'` ile başlatın. Aygıtları listelemek için `--input-device ''` kullanın. Çıkış varsayılan Windows aygıtıdır; `--output-device 'AYGIT ADI'` ile değiştirilebilir. Bağımsız sürüm giriş aygıtından aldığı sesi çıkışa işler; sistemde çalan tüm sesi kendiliğinden yakalamaz. Windows ses yönlendirmesi için DAW veya sanal ses aygıtı kullanılabilir.

## Ses motoru ve sınırlar

Çift hassasiyetli biquad filtreler, zıt low/high shelf tilt ve ayrı bass shelf. Kesimler 2/4/6/8/16 kutuplu Butterworth filtreleridir; her derecenin Q değerleri ayrı hesaplanır ve köşe frekansında −3.01 dB sağlar. Eğim değişiminde filtre hafızaları sürekli çalışır; çıkışlar yaklaşık 20 ms çapraz geçişle karışır. Parametreler, Analog, Saturation ve bypass da yumuşatılır. Kesimlerin OFF ucuna yakın bölgede katkıları yumuşakça sıfırlanır. Filtre frekansları örnekleme hızının %45'iyle sınırlanır; düşük örnekleme hızlarında fiziksel Nyquist sınırı geçerlidir. Sıfır bildirilen gecikme. İşleme sırasında kilit, dosya erişimi veya bellek ayırma yoktur.

### Analog ve Saturation

Machine/Source/Effects/Equalizer içindeki Luftikus uyarlaması incelendi. Özgün Analog modu doğrusal filtrelere gürültü ve tolerans ekler; dalga şekli doygunluğu içermez. F EQ, özgün 0.08 dB değişim eşiğini, ±0.04 dB rastgele kazanç hatasını, 1e-5 × (rastgele − 0.5) gürültü ölçeğini ve JUCE Random'ın 48 bit üretecini kullanır. Luftikus'un altı paralel bandı yerine F EQ'nun üç seri shelf bandına uygulanır; gürültü ayrıca shelf çıkışına eklenir. Bu nedenle tüm Luftikus EQ'sunun birebir ses kopyası değildir. Analog açıkken sessiz girişte çok düşük seviyeli gürültü duyulması beklenir. Stereo kanallar bağımsız rastgele akış kullanır.

Saturation ayrı, simetrik tanh doygunluğudur. Yalnız doğrusal olmayan bileşene birinci dereceden antiderivative antialiasing uygulanır; temiz bileşen geciktirilmez. Alias azalır, tamamen yok olma iddiası yoktur. Analog ve Saturation birbirinden bağımsız kullanılabilir. Her ikisi varsayılan kapalı/sıfırdır; eski 0.1.0 oturumlarında otomatik sıfırlanır, eğimler 12 dB/oktav olur. Eklenti kimlikleri eski oturum uyumluluğu için korunmuştur; görünür adı ve dosyaları F EQ'dur.

Bu bağımsız bir uygulamadır; TAL ürünü değildir. TAL'ın kaynak algoritması veya ölçülmüş transfer eğrileri bulunmadığından özgün TAL-USE ile birebir ses eşitliği iddiası yoktur. Boost ayarları çıkışta 0 dBFS'yi aşabilir; Volume ile seviye ayarlanabilir.

## Derleme

Rust 1.97.1 ve Visual Studio C++ Build Tools gerektirir. PowerShell:

```powershell
./build.ps1 -Test -PackageSource
```

Bu bilgisayarda Model F'nin yerel Rust araç zinciri otomatik bulunur. Kaynak paketine tüm git bağımlılıkları sabit sürümleriyle dahil edilmiştir; crates.io bağımlılıkları Cargo.lock ile sabitlenir. Önbellek hazırsa `-Offline` kullanılabilir.

## Doğrulama

DSP testleri: nötr şeffaflık, tilt yönü ve 900 Hz pivot, 300 Hz bass shelf, beş eğimin köşe seviyesi ve bastırması, gerçek sinüs sinyaliyle 96 dB/oktav bastırma ölçümü, eğim geçişleri, 22.05/44.1/48/96/192 kHz'de uç ayar kararlılığı, stereo ayrımı, Analog gürültü/tolerans sınırları, saturation harmonikleri ve tüm işlemleri kapsayan tam dry bypass. Eski oturumların yeni kontrolleri sıfırlaması da test edilir.

CLAP host entegrasyon testi: ses girişi/çıkışı, örnek 96'da otomasyonun erken başlamaması, +6 dB kazanç, bypass ve bütün parametrelerin oturum kaydıyla geri yüklenmesi. Arayüz aynı çizim kodundan PNG olarak üretilip görsel kontrol edilir. VST3 ve EXE derlenir; kullanıcı DAW'ında canlı dinleme testi yapılmamıştır.

## Lisans

GPL-3.0-or-later; LICENSE ve THIRD-PARTY-NOTICES.txt dosyalarına bakın. Luftikus Analog davranışı: Copyright 2019 lkjb, MIT; Luftikus-LICENSE.txt dağıtıma dahildir. Model F'den alınan pencere bağlayıcısı ve test altyapısı aynı lisans kapsamında kullanılır. NIH-plug ISC, baseview MIT/Apache-2.0, VST3 bağları GPL, URW Gothic font lisansı ve üçüncü taraf bildirimleri kaynak pakette korunur. Ek değişiklikler: third_party/EQ-NOTES.md.



## Yeni geliştirme konumu

`Codex/Music/F-EQ` içinden çalışın. [Türkçe geliştirici ve hata müdahale rehberi](docs/GELISTIRICI_REHBERI_TR.md). Eski mutlak build yollarını yeniden kullanmayın.

## Build from the complete Source ZIP

Extract the matching Source ZIP, enter the extracted directory and install Rust 1.97.1 plus the platform's C/C++ linker/SDK. Dependencies are included in `vendor/` and selected by `.cargo/config.toml`.

```sh
cargo test -p simple-eq --release --locked --offline
cargo build -p simple-eq --lib --release --locked --offline
cargo build -p simple-eq --example simple-eq --features standalone --release --locked --offline
```

For macOS Universal install both Rust targets, then use `python3 scripts/rust_audio_release.py F-EQ` to build, validate, sign ad-hoc and package. macOS 11+ deployment is selected by the script. The included `ci/rust-audio-release.yml` records the original build job. The `BUILD-REVISION.json` file identifies the exact source commit; `OFFLINE-SOURCE-VALIDATION.json` confirms all dependency manifests resolve inside the source distribution.
