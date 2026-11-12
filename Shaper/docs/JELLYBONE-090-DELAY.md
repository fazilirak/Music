# Jellybone 0.9.0 — Delay ve A/B bakım notları
Fazıl Irak · 12.11.2026

## Delay nasıl kullanılır?
Rack içindeki + menüsünden **Delay** ekleyin. Yeşil kontroller ve alttaki ortak eğri editörü kullanılır.
12 pattern, çizim araçları, sequencer ve MIDI/audio tetikleme diğer eğri efektleriyle aynıdır.
Üstteki nota bölünmesi Delay tekrar süresini belirler; eğri editöründeki süre ise modülasyon döngüsüdür.
Free modunda Time ms kullanılır. Host tempo bildirmiyorsa Tempo alanı devreye girer.
Stereo veya Ping-pong seçilebilir. Ortak bant paneli ve Stereo / dB / Mix paneli korunur.

Hedef menüsünden Send / Input, Mix, Feedback veya Filter seçin. Her hedef kendi Curve target,
Min, Max, Depth ve Smooth ayarlarını saklar. Birden fazla hedef aynı anda etkin olabilir.
Curve target kapalıyken o hedef Max değerinde sabit kalır. Depth=0 da Max değerini korur.
Min ve Max ters çevrilerek aynı eğri ters yönde kullanılabilir. Filter aralığı 80 Hz–18 kHz,
logaritmiktir; örnekleme hızının izin verdiği sınır otomatik uygulanır.

- **Send:** Delay belleğine yeni ses girişini ayarlar. Sıfır olduğunda önceki tekrarlar sürer.
- **Mix:** Kuru ve gecikmiş ses karışımını değiştirir; sıfırda kuru ses duyulur. Send'den farklı
  olarak bellekteki tekrarları da geçici olarak sessizleştirir.
- **Feedback:** Tekrar miktarını belirler. Güvenlik için gerçek kazanç en fazla %94'tür;
  gösterilen %100 bu güvenli üst sınıra karşılık gelir. Dönüş yolunda yumuşak sınırlandırma vardır.
- **Filter:** Feedback yolundaki düşük geçiren filtrenin frekansıdır. Koyulaşan tekrarlar sağlar.

### Dört örnek
1. **Cümle sonu throw:** Send Curve açık, Min=0, Max=1, son kısımda yükselen eğri; Mix sabit.
2. **Vuruş arası echo:** Send sabit, Mix Curve açık; vuruşta düşük, aralarda yüksek eğri.
3. **Feedback yükselişi:** Feedback Curve açık, Min=.15 Max=.75; son kısımda yükseltin.
4. **Koyulaşan tekrarlar:** Filter Curve açık, Min=.1 Max=.9; düşen eğri kullanın.

### Kapsam ve kaynak
QDelay 1.2.3 commit: 3130de7bbbc676b1c69b4dcfc8baa63851b57035. QDelay'ın DelayLine ve ZDF SVF bileşenleri kullanılır;
yeni eğri/Send/feedback yönlendirmesi Jellybone'a özgüdür. Bu modül QDelay'ın bütün işlemcisini
veya bütün presetlerini kopyalamaz. Reverse, diffusion, pitch shifting, ducking ve wow/flutter
bu ilk sürümde eklenmedi; mevcut Reverb, Pitch, Saturation ve harici FX ile zincirlenebilir.
Delay Time doğrudan eğri hedefi değildir. Süre değişiminde 150 ms yumuşatma ile tape tarzı perde
kayması kullanılır; perdeyi koruyan iki okuma kafalı çapraz geçiş sonraki geliştirme seçeneğidir.

### Bakım haritası
- Source/DelayEffect.*: parametreler, Send/Feedback/Filter DSP, state ve kompakt kontroller.
- scripts/prepare_sources.py: delaycurve namespace ve değişmemiş QDelay çekirdeğini üretir.
- upstream/qdelay: özgün kaynak; doğrudan değişiklik yapmayın.
- upstream-lock.json: eşleşen commit. C++17 uyumlu bileşenler seçildi; QDelay'ın C++20
  uygulama/arayüz katmanı derlemeye dahil edilmez.
- Tests/DelayTests.cpp: kuru uç, kuyruk, feedback, stereo, örnekleme/blok, state ve görüntü.
- Source/Embedded/RackAsync.h: kapanmış editör callback'lerinin atlanması.
- Source/Processor.cpp: callback/state kilidi ve atomik A/B banka değişimi.

A/B banka değişimi Delay parametre/eğrisini saklar; delay tamponundaki geçici ses geçmişi
preset dosyasına yazılmaz. Instance yeniden açıldığında kuyruk yeniden başlar. Rack bypass
esnasında motor işlemeye devam eder; tekrar etkinleştirmede kuyruk korunur.
