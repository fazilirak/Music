# F Saturator 0.4.0 — Character halkaları

**Geliştirici: Fazıl Irak · Düzenleme: 24.10.2026**

Bu sürümde dört kontrol grubu vardır: COLOUR → SATURATION → CHARACTER → OUTPUT.
Yeni grup, mevcut arayüzün turuncu dış / kırık beyaz orta / koyu iç halka tasarımını kullanır.

| Halka | Parametre | Aralık ve kullanım | Ses motorundaki karşılığı |
|---|---|---|---|
| En iç | Character | −100% Warm, 0% nötr, +100% Sizzle | İki nonlinear katın sertliğini birlikte azaltır/artırır. Sizzle daha belirgin üst harmonikler, Warm daha yumuşak kırpma sağlar; ayrı Tone EQ'sunu değiştirmez. |
| Orta | Harmonic Balance | −100% Even, 0% orijinal denge, +100% Odd | Even tarafı katların asimetrisini artırır; Odd tarafı statik eğrilerin çift simetrik bileşenini azaltır. Bu, girişteki tüm çift/tek harmonikleri ayıran bir spektral filtre değildir. |
| Dış | Analog Drift | 0% kapalı, 100% en fazla hafif sapma | Stereo bağlantılı 0.17 / 0.113 Hz değişimler, sürüşü en çok yaklaşık ±0.3 dB ve biası ±0.012 etkiler. Pitch, wow/flutter, hiss veya rastgele gürültü eklemez. |

Halkayı dikey sürükleyin veya üzerinde tekerlek kullanın. Shift hassas ayar, Ctrl/Alt+tıklama ya da çift tıklama varsayılana dönüş içindir. Etiketlere çift tıklayarak yüzde girilebilir; Character ve Balance için **eksi/arti işaretli** değer kullanılır. Merkez etiketler uç yönlerini belirtir.

## Uyumluluk ve bakım

- Eski 13 parametrenin kimlik/sırası, plugin ve bundle kimlikleri korunur. `character`, `harmonics`, `drift` liste sonuna eklenmiştir; yeni DAW otomasyonları ayrı alanlardır.
- Eski preset/DAW kaydında yeni alanlar yoksa üçü de sıfıra döner. Fabrika presetleri de yeni kontrolleri nötre sıfırlar. Yeni kayıtlar state sürümü 2 kullanır.
- Nötr üçlü eski 0.3.0 sesini korur. Yeni kontroller 25 ms rampayla ilerler; modelin 20 ms geçişi ve bildirilen latency değiştirilmez.
- Auto Gain hâlâ nötr karakterdeki referans sinüse kalibredir; yeni şekillendirme ayarlarında algısal eşit ses yüksekliği garanti etmez. Çıkışı kulakla/metreyle dengeleyin.
- `PluginEditor.cpp`: dört sütun / halka sırası. `ConcentricKnob.h`: signed yüzde, etiket ve hit-test. `PluginProcessor.cpp`: parametreler, preset ve state göçü. `SaturationDSP.cpp`: ADAA simetrisi, hardness ve drift. `Tests/DSPTests.cpp`: spektral, durum ve arayüz regresyonları.
- Üçüncü taraf kaynaklar ve lisanslar değiştirilmemiştir. Referans görselden kaynak kod veya marka tasarımı kopyalanmamıştır.

## Doğrulama

Mevcut DSP testlerine ek olarak: beş modelde Even/Odd H2 farkı, Warm/Sizzle H3 farkı, yeni uçların alias/stabilite testi, drift blok bağımsızlığı, mono/stereo eşitliği ve sessizlik kuyruğu, yeni state round-trip / eski state nötrleşmesi, değişmeyen DAW indeksleri, 12 halka hedefleme ve üç pencere boyutunda çakışmayan yerleşim. Test kodları 17–25 bu alanlara aittir.

Windows yerel derlemesi ve testleri `scripts/build-windows.cmd` ile çalışır. Windows ve macOS Universal paketleri aynı kaynak revizyonundan GitHub Actions ile derlenir. Platformlara ait test günlükleri ve eşleşen kaynak ZIP’leri sürüm paketindedir. Gerçek DAW ve Intel Mac donanım denemeleri ayrı yapılmalıdır.
