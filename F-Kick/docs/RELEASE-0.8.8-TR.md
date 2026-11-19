# F-Kick 0.8.8 — yerel doğrulama
Fazıl Irak · 19.11.2026 · Windows x64 / Release

## Tamamlanan teknik kontroller
- VST3, Standalone ve test hedefi derlendi. Son CTest 1/1 geçti (36.50 saniye); kayıt CTest-088.log.
- pluginval strictness 10 son ikilide SUCCESS: pluginval-0.8.8.txt.
- MIDI/Loop/Reverse/Tracking/LFO Sync gibi boolean parametrelerin hosta kesirli değer döndürmesi, state geri yüklemede pluginval hatası oluşturuyordu. BinarySwitchParameter mantıksal durumun 0 veya 1 olarak okunmasını sağlar. Parametre kimlikleri, preset v1 sözleşmesi ve DSP korunur. Kesirli giriş/state regresyonu eklendi.
- Gerçek derlenmiş VST3 ile 44.1/48/96 kHz × 64/256 buffer × 1/4 instance: 12 koşul. MIDI, üç parametre otomasyonu, 135→155 BPM ve state geri çağırma çalıştırıldı. On iki Rumble reçetesi yeni VST3 instance içinde state üzerinden geri açılıp ses üretti.
- Önceki DSP/preset/128 ses/Classic eşitliği/transient sıfır bypass/native OLE drop/GUI testleri geçti. Güncel FX editör görüntüsü incelendi; kompakt 912×511 tasarım korunuyor.

## CPU/gecikme bulgusu ve sınırları
HOST-ACCEPTANCE-2026-11-19.csv son koşunun tamamını içerir. Her koşul üç sanal ses saniyesidir. 48 kHz/256/4 instance ortanca callback 0.2878 ms, p95 0.3817 ms; blok süresi 5.3333 ms. Raporlanan algoritmik gecikme tüm koşullarda 0 örnek.
96 kHz/64/4 instance koşulunda 0.6667 ms bütçeyi aşan 30 callback ölçüldü (maksimum 4.3809 ms). Bu çevrimdışı duvar saati ölçümüdür; işletim sistemi zamanlamasını içerir ve ASIO underrun veya DAW CPU yüzdesi ölçümü değildir. Diğer 11 koşulda aşım görülmedi. Bu sonuç tüm sistemlerde gerçek zaman garantisi vermez.

## Canlı doğrulama
Kullanıcı Explorer'daki Classic - Concrete Hall.wav dosyasını açık Standalone grafik alanına bıraktı. Ekranda dosya adı, 4.00 s süresi ve yüklenen dalga şekli doğrulandı. Bu canlı deneme son 0.8.8 derlemesinden önceki 0.8.7 Standalone'da yapıldı; drop kodu bu güncellemede değişmedi. Son ikilide native drop testleri ayrıca geçti.
Ableton Live 12.4.16 içinde F-Kick track 1'e yüklenip arayüzü açıldı. Kurulu eklentinin sürümü doğrulanamadı. Sonraki MIDI/otomasyon/çoklu instance/oturum kaydı adımları bilgisayar aracının 'user input was detected in this window' ve pencere odak hataları nedeniyle tamamlanmadı. Kullanıcı 'Ableton hazır, kullanmıyorum' dedikten sonra hata tekrarlandı. Ableton/FL Studio kabul testi tamamlandı sayılmaz.

## Kalan kullanıcı kabulü
- 12 reçeteyi kuru/ayrı dönüş/birlikte dinleme, groove ve mono faz değerlendirmesi.
- Gerçek DAW'da güncel 0.8.8 ile MIDI/otomasyon/çoklu instance/oturum kaydet-aç ve cihaz gecikmesi kontrolü.
- Explorer→Standalone metin alanı ve Explorer→Ableton waveform/metin alanı fiziksel bırakma kontrolü.

Windows ikili ve eşleşen kaynak paketi yerel teslim içindir. GitHub/web/abonelere yayın yapılmadı; macOS/Linux bu görevde derlenmedi.
