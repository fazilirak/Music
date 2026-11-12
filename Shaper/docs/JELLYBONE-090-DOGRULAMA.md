# Jellybone 0.9.0 doğrulaması
Fazıl Irak · 13.11.2026

## Otomatik testler
Windows x64 Release, Visual Studio 2026 / MSVC, JUCE 8.0.7, C++17.
CMake + Ninja ile Standalone ve VST3 oluşturuldu. CTest altı hedef:
audio_integration, irak_dsp, pitch_dsp, curve_effects, utility_effects, delay_effect.
Kesin son tur çıktısı dağıtımın test-results.txt dosyasıdır.

### A/B regresyonu
İlk stres testinde queued bildirimlerin tesliminde access violation yeniden üretildi.
Eski editörlerde ham this yakalayan GUI callback'leri ve kapanmış işlemci/sequencer bildirimleri
bulundu. UI SafePointer ve işlemci weak lifetime korumasından sonra aynı stres testi geçti.
44 editör/banka değişimi, 30 eşzamanlı audio/state değişimi, kurulu F-Saturator VST3'üyle
12 A/B state/audio değişimi son testlerde başarılıdır. Değişik rack varlığı/gain değerleri ve
iki bankanın DAW oturumundan geri çağrılması ayrıca assert edilir.

### Delay
9 örnekleme/blok kombinasyonu; Mix=0 kuru uç, Send=0 eski tekrarların sürmesi,
yüksek feedback kararlılığı, ping-pong sağ kanal enerjisi, sıfır feedback tekrar kesilmesi,
120/240 BPM sekizlik nota süreleri, 6 kHz test sinyalinde düşük/yüksek filtre farkı,
hedef/pattern ve rack state kaydı ile editör görüntüsü doğrulandı.
Dış görünüm dist/preview-delay-090.png'dir. dist/Delay-090.shaper bir test presetidir.

## Gerçek DAW kontrolünün sınırı
Ableton Live 12 Standard'da kurulu 0.9.0 VST3 açıldı. F-Kick sonrasında Jellybone,
Volume 1 (A) ve Volume 2 (B) eğrileriyle denendi. A/B'de eğrilerin geri geldiği görüldü;
transport açık ve kapalı temel geçişlerde çökme olmadı. Gerçek klip sesi/uzun oturum,
Delay menüsü/tüm hedeflerin canlı kullanımı ve büyük proje CPU profili tamamlandı sayılmadı.
Windows otomasyonunda modal menü/dosya seçici odağı tutarsız olduğu için canlı Delay kontrolü
bu aşamada otomatik ses testleriyle karıştırılmadı. FL Studio kurulu bulunmadı.

## Sürüm ve kurulum
Test öncesi kurulu VST3 yedeği: dist/backups/Jellybone-before-090.
Canlı Ableton oturumu test esnasında açılmış 0.9.0 derlemesini kullanır. Son dağıtım ZIP'i
kaynak ve son ikiliyle eşleşir; test DAW'ı kapatılarak son paketten kurulum yapılmalıdır.
Bu görevde GitHub/web yayını veya mail gönderimi yapılmadı.

## 13.11.2026 — Delay paneli yerleşim düzeltmesi
Panel yüksekliği 140 yerine 124 piksel; açıklama satırı panelin içinde tutuldu. Paint/Seq/Curve satırıyla 8 piksel boşluk bırakıldı. Windows VST3 ve Standalone yeniden derlendi; DelayTests geçti ve güncel ekran görüntüsünde butonların tamamen açık olduğu doğrulandı. Bu görsel değişiklik için diğer DSP testleri tekrarlanmadı.


## 13.11.2026 — Dolu rack paleti
On bir efekt eklendiğinde gizli ekleme düğmesine ayrılan alan kaldırıldı. Kartlar, aralarındaki 10 piksel boşluk korunarak paletin tamamına eşit dağıtılır; son kart sağ sınırda biter. Sürükleme belirteci gerçek kart sınırlarını kullanır. Windows derlemeleri ve DelayTests başarılı; dist/preview-full-rack-090.png görsel olarak doğrulandı.

