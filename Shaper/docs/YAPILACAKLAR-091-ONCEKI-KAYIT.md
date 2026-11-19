# Jellybone Yapılacaklar
Fazıl Irak · 13.11.2026

## Bekleyen canlı doğrulamalar
- [ ] **FL Studio:** bu bilgisayarda kurulu/erişilebilir FL Studio bulunmadı. Sağ alt köşeden
  orantılı boyutlandırma, yedi Zoom, tıklama alanları, A/B ve yeniden açılışta ölçü/state testi
  gerçek FL Studio içinde yapılacak. Otomatik Zoom/state testleri geçti; bu canlı test yerine geçmez.
- [ ] **Delay kapsamlı DAW kontrolü:** yeni Delay'i gerçek ses/MIDI klipleriyle Ableton/FL Studio'da
  uzun oturum, render kuyruğu, farklı buffer/tempo ve CPU yükü altında doğrula.
  Otomatik ses testleri geçti; bu madde canlı müzikal kullanım ve performans kontrolüdür.
- [ ] **Kullanıcının özgün A/B senaryosu:** düzeltme sonrası kullanıcının asıl Ableton projesinde
  ve kullandığı harici FX'lerle de doğrulama. Bu oturumun özgün rack ayrıntıları bilinmiyor.

## 0.9.0'da tamamlanan geliştirmeler
- [x] Editör kapanınca sırada kalmış GUI callback'lerini SafePointer ile iptal et.
- [x] Kapanmış legacy işlemci/sequencer callback'lerini zayıf ömür belirteciyle iptal et.
- [x] Ses işleme/state/A-B banka değişimi ve host oturum kaydını aynı kilit sözleşmesiyle koru;
  state yüklenirken audio thread beklemek yerine kuru geçer.
- [x] 44 editör/A-B geçişi, 30 eşzamanlı ses/state geçişi ve harici F-Saturator ile 12 A/B geçişi.
- [x] Ableton Live 12'de Volume 1/Volume 2 eğrileri arasında A/B geri çağırma;
  transport açık/kapalı temel kontrolünde çökme görülmedi. Sesli uzun oturum testi ayrıca bekliyor.
- [x] QDelay 1.2.3 commit 3130de7bbbc676b1c69b4dcfc8baa63851b57035 sabitlendi.
- [x] QDelay DelayLine / ZDF SVF bileşenleriyle native stereo / ping-pong Delay modülü.
- [x] Ortak eğri, 12 pattern, sequencer, MIDI/audio trigger ve kompakt yeşil kontrol düzeni.
- [x] Aynı eğriden Send, Mix, Feedback ve logaritmik Filter; bağımsız Min/Max/Depth/Smooth.
- [x] Send kapalıyken kuyruk devamı, güvenli feedback, yumuşatma, önceden ayrılmış tamponlar.
- [x] Tempo senkronu, nota bölünmeleri, fallback tempo, sekiz başlangıç eğrisi.
- [x] State v8; önceki efekt kimlikleri ve v7/öncesi oturum geçişleri korunur; eski rack'lere Delay eklenmez.
- [x] C++17 uyumlu seçilen DSP bileşenleri namespace'e alınır; QDelay C++20 GUI katmanı derlenmez.
- [x] Delay Time eğri modülasyonu ilk sürüme alınmadı; 150 ms tape tarzı süre yumuşatması kullanılır.
- [x] Reverse/diffusion/pitch/ducking/wow-flutter değerlendirildi; ilk sürümde ayrı Delay kontrolleri değildir.
- [x] Telif/lisans kayıtları, About, Türkçe kullanım/bakım belgesi ve eşleşen kaynak paketi.
- [x] Altı otomatik test grubu; Delay'de 44.1/48/96 kHz, 32/257/1024 örnek blok,
  kuru uç, Send kuyruğu, sıfır/yüksek feedback, ping-pong, tempo, filtre, state ve görüntü kontrolü.

Kullanım/bakım: docs/JELLYBONE-090-DELAY.md.
Test kanıtları: docs/JELLYBONE-090-DOGRULAMA.md ve dağıtım paketindeki test-results.txt.
Önceki isteklerin tam kaydı: docs/YAPILACAKLAR-090-ONCEKI-KAYIT.md.
0.8.7 tamamlananları: docs/YAPILACAKLAR-087-TAMAMLANAN.md.

Liste doğrulanmamış canlı testleri gizlemek için boşaltılmadı. GitHub/web yayını ve abone
maili bu yerel geliştirme görevinin kapsamında başlatılmadı.

## 0.9.1 — Rack bankası yenilendi (13.11.2026)
- [x] 160 rack presetini az düğümlü ve gerilimli özgün eğrilerle yeniden oluştur.
- [x] Volume'un toparlanma/susma tasarımını, Gate aksanlarını ve müzikal süreleri gözden geçir.
- [x] Saturation/Pitch'in örneklenmiş 97 noktalı hareketlerini kaldır.
- [x] Reverb Wet/Send nokta kayıt formatını düzelt; bankayı dışa aktar/geri yükle ve görselleri doğrula.
Kullanım ve bakım: docs/FACTORY-BANK-091.md.
