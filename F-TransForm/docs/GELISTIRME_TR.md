<!-- Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026 -->
# F-TransForm 0.2 geliştirme haritası

Geliştirici: **Fazıl Irak**. Uygulama kaynak dosyalarının başında isim, tarih ve dosya görevi bulunur. Üçüncü taraf lisans ve yazar bilgileri korunur.

| Dosya | Görev |
|---|---|
| Source/Techniques.h | Yedi makronun isimleri, tarifleri ve etiket konumları. |
| Source/TransformDSP.h/.cpp | Ortak çoklu efekt zinciri, katman eğrileri, FIR, phase ve feedback. |
| Source/PluginProcessor.h/.cpp | Host tempo/bus/parametre ve şemalı oturum kaydı. |
| Source/PluginEditor.h/.cpp | Üç knob, metinler, dikey ortalı noktalar ve metreler. |
| Tests/Checks.cpp | Sayısal ses kontrolleri, sentetik techno loop ve editör PNG. |
| scripts/build-windows.cmd | Windows Release ve CTest. |
| scripts/package-windows.ps1 | Yerel VST3/standalone kopyası ve hash manifesti. |

## Mimari

Ana giriş → HP → drive → phase → paralel hall + çıkış HP / echo / frequency shift → gövde azaltma ve noise/riser → low-pass / DC temizliği / tepe koruması → kısa engage / bypass karışımı.

Tarif değiştirmek ayrı bir motor açmaz. Ortak motorun 11 hedefi 45 ms'de yeni tarife gider; hızlı seçici hareketi mevcut ara değerlerden devam eder. Bellekler prepare() aşamasında ayrılır, process() sırasında tahsis veya kilit yoktur.

Intensity 18 ms, karakter 50 ms ve bypass 20 ms yumuşatılır. Filtre katsayıları 16 örnekte bir yenilenir; sayaç blok sınırından bağımsızdır. Host otomasyonu blok başında okunur. Echo tempo değişiminde iki okuma kafası çaprazlanır; çok hızlı sürekli tempo otomasyonu gerçek DAW'da ayrıca dinlenmelidir.

## Uyumluluk

Parametre kimlikleri korunur: technique, intensity, character, bypass. Sol Choice artık 0..6 aralığındadır; eski 17 elemanlı normalize otomasyon yeni sesle birebir uyumlu değildir.

FTRANSFORM_STATE kökünde engineVersion=2 saklanır. Eski şemasız durumdaki teknik indeksi güvenli bir makroya taşınır; intensity, character ve bypass korunur. Eski ses algoritması geri getirilmez. Önceki motorun kaynak ZIP yedeği dist içindedir.

İkinci giriş bus'ı host düzeni için kalır, motor ikinci kaynağı kullanmaz. Preset bankası yoktur; tek boş program yuvası JUCE host sözleşmesini karşılar.

## Kontroller

- 7×5 sıfır yoğunlukta bit düzeyinde kuru geçiş.
- 22.05/44.1/48/96 kHz, mono/stereo ve karakter uçlarında sonlu/sınırlı ses.
- Yedi modda blok boyundan bağımsızlık ve birbirinden farklı ses çıktısı.
- Air Bloom düşük frekans azaltma ve Critical Mass tam son çözülme.
- Giriş kesilince noise/riser ve reverb/echo kuyruklarının sönmesi.
- DAW durum kaydı, eski şema dönüşümü, bypass ve hızlı mod/tempo otomasyonu.
- Seçici aralığı, gerçek editör PNG çıktısı ve sekiz stereo WAV karşılaştırması.

Derle: scripts/build-windows.cmd. Ardından PowerShell 7 ile scripts/package-windows.ps1. Gerçek DAW taraması ve referans eklentiyle A/B dinlemesi otomatik testlerin kapsamı dışındadır.

