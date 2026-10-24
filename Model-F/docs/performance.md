# SuperSaw 8 nota — sürüm 0.1.4

SuperSaw en fazla 8 nota çalar. Her notada yedi saw osilatörü, ayrı ladder filtre ve contour/VCA vardır. En fazla 56 saw osilatörü çalışır; devre LFO'ları ayrıca çalışır. MONO başlangıç modu olmaya devam eder. POLY ve RING POLY dört notadır.

Boş sesler render edilmez. Tuş bırakıldıktan sonra sönmekte olan sesler işlenmeye devam eder. 8 ses dolarsa önce bırakılmış en eski ses, hepsi basılıysa basılı en eski ses alınır. Çalarken bellek ayırma/serbest bırakma yapılmadığı otomatik testle doğrulanır.

## Performans

Daha yüksek sınırlar denenerek 32 sürekli notanın bu bilgisayarda gerçek zaman bütçesini aştığı, 16 notanın da daha dar işlemci payı bıraktığı görüldü. Kullanıcının güvenli tarafta kalma tercihi doğrultusunda sınır 8 olarak belirlendi. Bu son sürümde 32 nota seçeneği yoktur.

Son sürümün 48 kHz stereo, 128/256/512 sample buffer ölçümleri `supersaw-load.json` dosyasındadır. Dört yardımcı iş parçacığı, %50 detune, sürekli basılı notalar kullanılır. Bu bilgisayar: Intel Family 6 Model 186, Windows, 20 mantıksal işlemci. Sonuçlar toplam işlemci kullanımı değil, ses bloğu süresinin kullanılan kısmıdır. Ölçüm bütün seslerin bitmesini bekler; işi zaman sınırında atlamaz.

Sample rate, preset, diğer eklentiler ve bilgisayarın güç/sıcaklık durumu performansı etkiler. Bu ölçüm manuel DAW testi veya her bilgisayarda kesintisiz çalma garantisi değildir.

Envelope filtre ve Volume hedefleriyle sekiz notalık SuperSaw ölçümleri mseg-filter-load.json ve mseg-volume-load.json dosyalarındadır. Loop, Amount %30 kullanılmıştır. Son ölçümde 48 kHz / 256 sample Volume hedefi ortalama 1.55 ms (%29) aldı ve süre aşımı görülmedi. Sonuçlar bilgisayarın o anki yüküne göre değişir.

Testler; sekiz ayrı sesin duyulmasını, dokuzuncu notada ses seçimini, çalınan seste eski notanın note-off mesajının etkisiz kalmasını, release sonunda seslerin boşa dönmesini ve MIDI PANIC davranışını kapsar. SuperSaw Key/Pad presetlerinin seviyeleri, akorlar çalınırken diğer hazır seslerle uyumlu kalacak şekilde yeniden ayarlanmıştır. Kullanıcı preset dosyaları değiştirilmez.

Ölçümü tekrar çalıştırmak için:

```powershell
cargo run -p model-f --example supersaw-load --release -- docs/supersaw-load.json
```


## 0.2.0 — 24.10.2026, Fazıl Irak

Kararlı mono blokta kilit/gate/gain sorguları blok başına alınır; mixer katsayıları önbellekte tutulur. Yardımcı işçiler 200 yerine 50 µs aktif bekleyip park olur. Kalite, oversampling ve nota sınırları aynı kalır.

Aynı 18 senaryolu yerel 48 kHz / dört işçi testinde süreç CPU süresi 10578.125 ms → 9421.875 ms oldu (yaklaşık %10.9 azalma). 128/256/512 sample ve 0/1/3/4/6/8 nota senaryolarında sıfır blok süresi aşımı gözlendi. Bunlar toplam işlemci yüzdesi değildir; süreç CPU zamanı ve ses bloğu bütçesidir. Tek karşılaştırma, yük/güç/sıcaklık değişkenlerine bağlıdır ve başka bilgisayarlarda aynı kazancı garanti etmez.

Beş tek iş parçacıklı örnek senaryosu ölçüldü; dört eski referansın PCM hash'i birebir aynı kaldı. Ham raporlar 020-*.json dosyalarındadır. Bu senaryolarda wall-clock süre farkları küçük/değişkendir; kalite düşürülerek elde edilmiş bir hızlandırma yoktur. Multi/FM/Sync yeni ses tasarımları daha çok hesap gerektirebilir.
