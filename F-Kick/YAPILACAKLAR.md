# F-Kick — Yapılacaklar
Fazıl Irak · 19.11.2026 · Yerel sürüm 0.8.8

## Tamamlananlar
- [x] Gerçek VST3 test hostunda MIDI, otomasyon, tempo geçişi, 1/4 instance ve state kaydet/geri aç matrisi.
- [x] On iki Classic/Groove/Hybrid reçetesinin yeni VST3 instance içinde geri çağrılması ve ses üretmesi.
- [x] Boolean host/state readback düzeltmesi; pluginval 10 ve tüm CTest regresyonları.
- [x] Kullanıcı yardımıyla Explorer→Standalone grafik alanına dosya bırakma ve dalga/dosya adı doğrulaması (canlı 0.8.7; 0.8.8 native testleri de geçti).
- [x] Yerel Windows VST3/Standalone ve eşleşen kaynak paketi.

## Bekleyen kullanıcı/DAW kabulü
- [ ] On iki reçetenin ayrı ve birlikte kulakla kalite, groove ve mono faz kontrolü.
- [ ] Güncel 0.8.8 ile gerçek Ableton/FL Studio MIDI, otomasyon, çoklu instance CPU/gecikme ve oturum kaydet-aç denemesi. VST3 test hostu tamamlandı; canlı DAW yerine sayılmaz. Araç odak/kullanıcı girişi hatası nedeniyle canlı kontrol sürdürülemedi.
- [ ] Explorer→Standalone metin alanı ve Explorer→Ableton waveform/metin alanı fiziksel sürükle-bırak denemesi.

Kanıt ve ölçüm sınırları: validation/TEST_SONUCLARI_TR.md. 96 kHz/64 buffer/4 instance çevrimdışı koşulundaki 30 zaman aşımı CSV ve raporda korunur. GitHub/web yayını ayrıca talep edilmelidir.
