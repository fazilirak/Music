# Model F 0.2.4 — CPU incelemesi

Fazıl Irak · 26.10.2026

## Önce / sonra: aynı ses, aynı makine

0.2.3 referans executable'ı ve 0.2.4 Release/Thin-LTO executable'ı beşer kez çalıştırıldı. Çalıştırma sırası dönüşümlü değiştirildi. Aynı seed=42, Windows x64, 48 kHz, blok=256, ses=96.000 örnek (2 saniye), worker=0. Tabloda beş çalışmanın medyan süreleri var. Plus değişim daha kısa işlem süresidir. Örneklerin f32 bit hash'leri bütün tekrarlarda eşittir.

| Senaryo | Önce ms | Sonra ms | Süre azalması | Ses bit-eşit |
| --- | ---: | ---: | ---: | --- |
| mono-idle | 104.990 | 101.189 | +3.62% | Evet |
| mono | 127.426 | 125.002 | +1.90% | Evet |
| mono-filter | 127.408 | 124.995 | +1.89% | Evet |
| poly | 515.992 | 498.665 | +3.36% | Evet |
| supersaw | 641.185 | 642.408 | -0.19% | Evet |

Sonuç: Mono/Poly'de küçük kazanım var; SuperSaw'da anlamlı bir hızlanma ölçülmedi. Bu tablo işletim sistemi CPU kullanım yüzdesi veya tüm projelerde performans garantisi değildir. Background uygulamalar, frekans yönetimi ve host zamanlaması sonuçları etkileyebilir.

## Altı karakter matrisi

`characters-024/character-performance.json` 108 koşul içerir: 6 karakter × Mono/Poly/SuperSaw × 44.1/48/96 kHz × 64/256 blok. Her senaryo hazırlık/rampa dışında yarım saniyelik sesi tek worker ile işler. VA Stack seçimi her zaman yedi saw bankını açtığından, bu karakterin Mono etiketli satırı da SuperSaw bankıdır; `case` istenen senaryoyu, karakterin gerçek davranışı rehberi izler. Sayıları gerçek host CPU göstergesiyle aynı kabul etmeyin.

Yeni filtre yolları aynı sesin hızlandırılmış kopyaları değildir; farklı ses üretirler. Bu yüzden yeni bir karakterin daha kısa süresini, Ladder'ın sesini koruyarak elde edilmiş CPU kazancı olarak sunmuyoruz. Karşılaştırma için üstteki bit-eşit tabloyu kullanın.

## Uygulananlar

- Değişmeyen filtre kontrol düğümü, tam bit anahtarı ve kalite kipiyle cache edilir. Kontrol girdisi değişince sonuç yeniden hesaplanır.
- Poly seslerinin sabit pan kazançları örnek döngüsünün dışında hesaplanır.
- Yalnız seçilen karakter çalışır; 12 ms geçişte iki filtre kısa süre çalışır. Audio callback'e allocation veya dosya erişimi eklenmedi.
- Ses algoritmasını değiştirmeyen Thin LTO / tek codegen unit Release optimizasyonu kullanılır. Fast-math yoktur.
- UI yenileme üst sınırı 30'dan 20 FPS'e indi. Ses ve otomasyon örnekleme hızı azaltılmadı.
- Sabit osilatör Drive yapısını kopyalayan deneysel cache tutarlı kazanç vermediği için nihai kaynakta tutulmadı.

## Doğrulama

81 ürün testi: 64 unit, 8 karakter/akort/preset/geçiş, 4 CLAP host, 1 MSEG audio, 3 overload audio ve 1 gerçek zamanlı allocation testi. Buna ek olarak yeni çekirdeğin 2 testi, 8/44.1/48/96/192 kHz uç değerleri ve SVF frekans ayrımını sınar. Önceki 5 referans ses hash'i korunur. Panel 1× ve 2× SVG/piksel testleriyle kontrol edilir. Gerçek Ableton ve Mac donanım denemesi bu sonuçlardan çıkarılamaz.
