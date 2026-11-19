# F-Saturator yapılacaklar

Proje sahibi: Fazıl Irak
Son düzenleme: 19 Kasım 2026
Kaynak: F-Saturator 0.5.5 kullanıcısının e-posta geri bildirimi.
Durum: 0.5.6 yerel Windows uygulaması ve otomatik doğrulamalar tamamlandı. Aşağıdaki dış kabul kontrolleri bekliyor.

## Arayüz ölçeği ve pencere boyutu

- [x] UI menüsünde %50, %75, %100, %125, %150, %175 ve %200 ölçek seçimi.
- [x] Seçilen ölçeği ve son pencere boyutunu kalıcı saklama; yeni processor/editör açılışında geri okuma.
- [x] Görünüm tercihlerini ses presetlerinden bağımsız tutma; ilk açılış, bozuk kayıt, kullanılabilir ekran alanı ve DPI/ekran alanı değişimine karşı boyut sınırı.
- [x] Orantılı sağ alt köşe boyutlandırması, yedi ölçekte yerleşim/görsel, preset/session bağımsızlığı ve editör yaşam döngüsü regresyonları; pluginval editör testleri.

## CPU kullanımı

- [x] Tek motorla tekrarlanabilir eski/yeni ölçüm: stereo, beş model, 44.1/48/96 kHz, 64/256 blok; aynı süreç/çekirdek, üç tekrar medyanı.
- [x] Geri bildirim sahibinin CPU, sample rate, buffer ve preset bilgilerini isteme. Henüz gelmedi; kullanıcı yerel çalışmaya devam edilmesini istedi.
- [x] ADAA sıcak yolundaki yinelenen hesapları önbelleğe alma, sıfır Drift maliyetini azaltma; GUI'deki sürekli tam repaint ve gereksiz transfer eğrisi hesabını ayrı inceleyip kaldırma.
- [x] Önce/sonra CPU ve ses karşılaştırması: 2.621.440 örnekte fark sıfır; kalite, otomasyon, preset/latency testleri geçti. Ayrıntılı ölçüm ve sınırlar docs/RELEASE-0.5.6-TR.md içinde.
- [x] pluginval seviye 10 sırasında saptanan aç/kapa parametresi geri yükleme tutarsızlığını düzeltme; regresyonla doğrulama.
- [x] Windows x64 VST3/Standalone derlemesi ve yerel dağıtım paketi.

## Dış kabul / geri bildirim bekleyenler

- [ ] Geri bildirim sahibinin Windows 11 / Ableton Live 12.4.3 sistemi ve sorunlu presetinde önce/sonra doğrulama. Buradaki Live 12.4.16; aynı ortam olduğu iddia edilmez.
- [ ] Ableton oturumunu tamamen kapatıp yeniden açarak ve fiziksel farklı DPI ekranları arasında taşıyarak canlı kabul. Canlı Standalone kontrolü Esc ile durduruldu; otomatik editör ve host testleri tamamlandı.
- [ ] Ayrı GUI/GPU zaman serisi profili gerekiyorsa gerçek DAW iş yüküyle ölçme.
