# Model F 0.2.4 — Karakterler ve genel akort

Fazıl Irak · Düzenleme: 26.10.2026

## Küçük karakter seçicisi

Preset alanının altındaki eski INIT / 160 FACTORY SOUNDS satırı kaldırıldı. Aynı alanda 22 piksel yüksekliğinde Ladder, Edge, Bite, State Variable, Acid ve VA Stack seçimleri var. Seçili karakter turuncu, diğerleri koyudur. Panel hâlâ 1120×760; osilatörler ve envelope bölümleri büyütülmedi. Sound Library veya Multi-Stage Envelope açıkken karakter seçimi kilitlenir.

Karakter seçimi preset ve DAW oturumuna kaydedilir. Eski dosyada karakter yoksa mevcut ses olan **Ladder** seçilir. Seçim bir EQ preseti değildir: aktif notanın filtre işlem yolu değişir. Sadece seçilen filtre çalışır; karakter değişimindeki 12 ms çapraz geçişte eski ve yeni yol kısa süre birlikte çalışır. Hızlı tekrarlanan seçimler, sürmekte olan geçiş bitince son hedefe gider.

| Karakter | Uygulanan davranış | Küçük filtre biçimi alanı |
| --- | --- | --- |
| Ladder | Önceki Model F'nin devre temelli osilatör, ladder filtre ve VCA yolu; yeni sürümün nötr karşılaştırmalarında aynı örnekler | Pasif |
| Edge | 12 dB/oktav TPT low-pass, asimetrik giriş sürüşü ve ayrı 80 Hz high-pass kutbu. Odyssey Rev-I yaklaşımı ses hedefidir; mevcut Sync/FM/Ring seçenekleri kullanılabilir | Pasif |
| Bite | Ayrı rezonanslı high-pass ve low-pass aşamaları, aralarında nonlinear sürüş | HP kesimi 20–2500 Hz |
| State Variable | İki kutuplu state-variable filtre; LP → Notch → HP → BP çıkışlarının karışımı | Fareyle dikey sürükleyerek çıkışlar arasında geçiş |
| Acid | Saw/pulse dalgalarında yumuşak asimetri, dört kutuplu nonlinear rezonanslı filtre, asimetrik giriş ve kat doygunluğu | Pasif |
| VA Stack | Mevcut yedi saw osilatörlü SuperSaw bankı otomatik açılır; daha açık dijital TPT filtre yolu | 12 ve 24 dB/oktav yollarının karışımı; ara konum ayrı bir tam sayı eğimi değildir |

Bu karakterler bağımsız geliştirilmiş algoritmalarla hazırlanmış ses hedefleridir. Belirtilen tarihî cihazların ölçülmüş, birebir devre veya firmware emülasyonları değildir; üretici kodu, logosu veya skin'i eklenmemiştir. Ladder dışındaki filtreler iki alt örnekli çalışır ve mevcut devrenin kontrol akımından key tracking, contour ve MSEG hareketini alır. Aşırı girişte yumuşak gerilim sınırlaması uygulanır.

## Hızlı kullanım

1. Preset seçip aynı notayı farklı karakterlerle dinleyin. Sadece daha yüksek olanı daha iyi sanmamak için ses seviyesini karşılaştırın. Referans notada yaklaşık eşleme yapılmıştır; her preset için loudness normalizasyonu değildir.
2. Edge'de Sync/FM veya Ring'i mevcut kontrollerle deneyin. Karakter seçimi bunları zorla açmaz.
3. Bite veya State Variable seçildiğinde filtre başlığındaki küçük alanı dikey sürükleyin. Bite'de HP frekansı, State Variable'da LP/Notch/HP/BP karışımı değişir. Shift hassas ayar, Ctrl/Alt-tıklama varsayılana dönüş sağlar.
4. Acid için Saw veya pulse, orta-yüksek Emphasis, filtre contour ve mevcut Glide ile başlayın. Osilatör şekli değişimi de 5 ms yumuşar. Bu sürümde TB-303 sequencer, accent veya ayrı bir 303 slide sistemi yoktur.
5. VA Stack seçildiğinde Voice alanı SuperSaw, OSC1 Saw × 7 gösterir. İkinci audio osilatör, Sync/FM ve dalga seçimi bu kipte pasiftir. Detune dağılımını mevcut Detune ile ayarlayın. Başka karakter seçince kayıtlı Voice Mode ve osilatör ayarları geri kullanılır; bu ayarlar üzerine yazılmaz. Bu mod JP-8000'in diğer dalga biçimlerini, efektlerini veya performans mimarisini yeniden üretmez.

## Global Tuning

Multi-Stage Envelope butonunun solundaki **GLOBAL TUNING 440.0 Hz** alanına basılı tutup yukarı/aşağı sürükleyin. A4 referansı 400.0–480.0 Hz aralığında, 0.1 Hz adımla değişir. Shift daha hassas sürükleme sağlar; Ctrl/Alt-tıklama 440 Hz'e döner. Nota, iki osilatör ve SuperSaw bankı aynı frekans oranıyla akort edilir; bu kontrol OSC1 Tune veya Detune yerine geçmez.

Bu değer ses presetlerine yazılmaz ve `.modelf` yüklerken değişmez; presetin değiştiğini belirten işareti de tetiklemez. DAW oturumunda ise korunur. Yani genel akort mevcut eklenti örneğine/oturuma aittir; tüm açık Model F örneklerini tek hamlede değiştiren sistem çapında ayar değildir.

## CPU ve sınırlar

- Yeni karakterlerin hepsi aynı anda hesaplanmaz. Yalnız kısa geçiş sırasında iki filtre etkin olur.
- Stereo pan kazancı poly render döngüsü dışına alındı. Tamamen aynı filtre kontrol düğümü için sonuç, kalite kipi dahil anahtarla önbelleğe alınır; yaklaşık eşik veya azaltılmış ses kalitesi kullanılmaz.
- Release derlemesinde Thin LTO ve tek codegen unit ile crate sınırlarında derleyici optimizasyonu etkinleştirildi. Fast-math kullanılmaz.
- Arayüz üst yenileme sıklığı 30 yerine 20 kare/saniyedir. Ses örnekleme hızı, zarf zamanlaması ve otomasyon hızı düşürülmedi.
- `CPU-024-TR.md` ve JSON raporları ölçüm koşullarıyla birlikte değerlendirilmelidir. Sentetik render süresi, Ableton'ın göstereceği kesin CPU yüzdesi değildir. VA Stack yedi osilatör içerdiği için tek osilatör kadar hafif olması beklenmez.

## Kaynakta neresi değişti?

- `params.rs`: sona eklenen character, character_shape ve global_tuning; eski host indeksleri korunur.
- `presets.rs`: genel akordu capture/apply/differs dışında tutma; eski dosyalarda yeni karakterleri nötr açma.
- `engine.rs`: genel akordu iki osilatörün cents ofsetine ekleme; yedi saw bankı merkez sürüşünü izler.
- `model-f-core/src/character_filter.rs`: seçilebilir filtrelerin hafızaları, TPT aşamaları ve sinyal işleme.
- `model-f-core/src/voice.rs`: filtre seçimi, çapraz geçiş, Acid dalga şekli ve tam anahtarlı kontrol cache'i.
- `panel.rs` / `editor.rs`: ortak hit-test/çizim geometrisi; küçük etiketler, dikey sürükleme ve modal kilitler.
- `tests/character_audio.rs`: gerçek ses, seviye eşlenmiş farklılık, preset/session, render akordu ve geçişler.
- `tests/realtime.rs`: karakter/tuning otomasyonu sırasında audio thread'de allocation/free olmadığının kontrolü.

## Referanslar

- Korg Odyssey özellikleri: https://www.korg.com/uk/products/synthesizers/arpodyssey/features.php
- Korg MS-20: https://www.korg.com/us/products/synthesizers/ms_20mini/page_1.php
- Roland JP-8000: https://cdn.roland.com/assets/media/pdf/JP-8000.pdf
- Mevcut SuperSaw bankının dağılım kaynağı ve lisans bildirimleri özgün kaynakta korunur.

Yerel Windows derlemesidir; bu çalışma kendi başına GitHub/web yayını veya Mac derlemesi yapmaz. Gerçek Ableton oturumu ve diğer host/donanım testleri ayrıca gereklidir.
