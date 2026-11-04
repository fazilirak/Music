# Model F 0.3.0 — CPU çalışması

4 Kasım 2026 · Yerel, tek iş parçacıklı offline render · 128 örneklik blok · sabit seed 42.

Her sürüm üç kez ölçüldü. Aşağıdaki değerler medyandır ve bir çekirdeğin kapasitesine oranı gösterir; bilgisayarın toplam CPU yüzdesi değildir. Canlı DAW ve çok iş parçacıklı kullanım ayrı koşullardır.

Yapılanlar:

- EG faz adımı ve LFO/Step artımları ayar değişiminde hesaplanıyor.
- Step yumuşatma katsayısının pahalı üstel hesabı örnek döngüsünden çıkarıldı.
- Preamp aktarımı sabitken önceki antiderivative sonucu yeniden kullanılıyor; MIC/LINE geçişinde önbellek geçersizleşiyor.
- Preset yüklemede değişmeyen parametreler yeniden gönderilmiyor.
- 1840 parametre heap üzerinde tutuluyor; kurucu geçici alanları ayrı fonksiyonda kalıyor. 512 KB yığın testi geçti. Ses işleme içinde yeni bellek ayırma/boşaltma eklenmedi.

24 senaryonun örnek bit dizisi özetleri önce/sonra eşleşti. Toplam CPU farkı küçük ve değişkendir; genel büyük CPU düşüşü iddia edilmez. Değiştirilmeyen Dry referansının ölçümü de koşullar arasında kaymıştır.

| Kip | Hz | Durum | Önce % | Sonra % |
|---|---:|---|---:|---:|
| Mono | 48000 | Dry | 5.93 | 6.31 |
| Mono | 48000 | 18 modulators | 7.14 | 7.13 |
| Mono | 48000 | 18 modulators + LINE | 7.81 | 7.46 |
| Mono | 48000 | 18 modulators + MIC | 7.74 | 7.51 |
| Mono | 96000 | Dry | 11.64 | 12.41 |
| Mono | 96000 | 18 modulators | 14.09 | 14.60 |
| Mono | 96000 | 18 modulators + LINE | 15.11 | 15.37 |
| Mono | 96000 | 18 modulators + MIC | 15.13 | 14.97 |
| Poly4 | 48000 | Dry | 24.18 | 25.55 |
| Poly4 | 48000 | 18 modulators | 28.66 | 29.68 |
| Poly4 | 48000 | 18 modulators + LINE | 28.61 | 29.81 |
| Poly4 | 48000 | 18 modulators + MIC | 29.11 | 29.86 |
| Poly4 | 96000 | Dry | 46.66 | 48.53 |
| Poly4 | 96000 | 18 modulators | 56.69 | 58.54 |
| Poly4 | 96000 | 18 modulators + LINE | 57.16 | 59.07 |
| Poly4 | 96000 | 18 modulators + MIC | 57.44 | 59.10 |
| SuperSaw8 | 48000 | Dry | 67.97 | 70.45 |
| SuperSaw8 | 48000 | 18 modulators | 78.65 | 79.16 |
| SuperSaw8 | 48000 | 18 modulators + LINE | 77.79 | 82.05 |
| SuperSaw8 | 48000 | 18 modulators + MIC | 78.88 | 80.96 |
| SuperSaw8 | 96000 | Dry | 133.07 | 137.93 |
| SuperSaw8 | 96000 | 18 modulators | 151.49 | 154.92 |
| SuperSaw8 | 96000 | 18 modulators + LINE | 153.44 | 157.38 |
| SuperSaw8 | 96000 | 18 modulators + MIC | 150.93 | 160.21 |
