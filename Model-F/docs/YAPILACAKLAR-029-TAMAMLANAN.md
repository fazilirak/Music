# Model F 0.2.9 — Değişiklik ve doğrulama

Fazıl Irak · 03.11.2026

## Tamamlanan geliştirmeler

- Node dalgası değiştirildiğinde Ctrl+sürükle fırçası aynı dalgayı kullanır. Çizilen hücrenin başlangıcı seçilerek Node etiketi doğru segmenti gösterir. Linear segment seçimi fırça dalgasını zorla değiştirmez; sekiz dalga seçiminin tümü test edildi.
- Kaynak incelemesi ve gerçek Engine render'larıyla çıkışın düşük olduğu doğrulandı. Referans patch'lerde peak yaklaşık -20 ile -33 dBFS idi. Osilatör/filtre devresini bozmak yerine ayrı Full çıkış kalibrasyonu eklendi.
- Full: +10 dB kazanç, 180 Hz tek kutuplu düşük shelf (DC'de +1.5 dB). Limiter, kompresör veya otomatik seviye eşleme yoktur. Çıkışta yeterli headroom bırakılmalıdır; her olası patch/Output maksimumu için clipping olmama garantisi verilmez.
- Full/Legacy geçişi 20 ms içinde yumuşar. Sıfır Output ve güç/panic susturması korunur. Audio callback'e bellek ayırma veya kilit eklenmedi.
- Yeni örnek ve fabrika preset seçimi Full kullanır. Eski DAW state'i ve eski kullanıcı preset'i alan yoksa Legacy yükler. Eski oturumda Full istenirse küçük OUTPUT: LEGACY düğmesine basılır.
- Eski parametre kimlikleri/sırası korunur; output_voicing en sona eklendi. Eski ses koruması aşağıdaki örneklerde bit düzeyinde doğrulandı.

## Testler

- model-f --lib: 72 test geçti (waveform olay yolu, preset/state geçişleri, görsel yerleşim, Full/mute 44.1/48/96 kHz dahil).
- Gerçek Engine, 48 kHz, sabit seed=42; Init/Bass/Lead/Pad × altı karakter = 24 ölçüm. Üç kayıt seti: audio-before-029, audio-after-029, audio-legacy-029.
- Aşağıdaki RMS artışı ham seviyeler arasındaki farktır; tek başına ses kalitesi ölçütü değildir. 180 Hz altındaki shelf farkı dışında devre motoru korunur.
- Sylenth1/Serum/UAD Minimoog ile aynı ayarlarda kontrollü render/dinleme karşılaştırması yapılmadı. Öznel ton onayı henüz yok. Ölçümlerden daha iyi ses kalitesi sonucu çıkarılmaz.
- Canlı GUI açma isteği Computer Use uygulama izni zaman aşımına uğradı. Kaynak testleri ve gerçek SVG panel render'ı kontrol edildi; canlı DAW kontrolü bekliyor.

## Ölçümler

| Ses / karakter | Eski peak dBFS | Full peak dBFS | RMS artışı dB |
|---|---:|---:|---:|
| Init / 0 | -20.65 | -10.91 | 10.40 |
| Init / 1 | -22.14 | -12.34 | 10.39 |
| Init / 2 | -22.41 | -12.72 | 10.40 |
| Init / 3 | -24.77 | -14.97 | 10.40 |
| Init / 4 | -23.03 | -13.23 | 10.40 |
| Init / 5 | -21.00 | -11.14 | 10.28 |
| Bass / 0 | -24.63 | -13.26 | 11.38 |
| Bass / 1 | -33.34 | -21.89 | 11.37 |
| Bass / 2 | -27.85 | -16.39 | 11.39 |
| Bass / 3 | -28.01 | -16.67 | 11.39 |
| Bass / 4 | -26.65 | -15.29 | 11.39 |
| Bass / 5 | -28.25 | -17.51 | 10.89 |
| Lead / 0 | -24.03 | -13.42 | 10.55 |
| Lead / 1 | -27.34 | -16.58 | 10.55 |
| Lead / 2 | -26.14 | -15.47 | 10.55 |
| Lead / 3 | -27.83 | -17.21 | 10.55 |
| Lead / 4 | -26.45 | -15.82 | 10.55 |
| Lead / 5 | -23.44 | -13.59 | 10.27 |
| Pad / 0 | -22.70 | -11.92 | 10.76 |
| Pad / 1 | -24.01 | -13.41 | 10.73 |
| Pad / 2 | -22.92 | -12.34 | 10.76 |
| Pad / 3 | -25.05 | -14.27 | 10.77 |
| Pad / 4 | -24.50 | -13.71 | 10.76 |
| Pad / 5 | -20.00 | -9.96 | 10.32 |

## Legacy eşitliği

- Init: aynı WAV baytları / SHA-256 1fc9bc0ee8439a17f2821270b27e0652ee9e634be14a595fccbeb08c132db133
- Bass: aynı WAV baytları / SHA-256 d0cb3edb6f0b8f023302bd02e2c613f86cb75cc05d69ad32d960ec25def25164
- Lead: aynı WAV baytları / SHA-256 e3a67d26d64057910bce3918d04e79f35d9cdb9bfc83a215b001a61719f1db62
- Pad: aynı WAV baytları / SHA-256 17c56c270ce53f2e067b4955a42061835ceff213c81dd12dac0c05a593e02577

## Dağıtım

Yerel Windows VST3, CLAP ve Standalone: dist/Model-F-Windows-0.2.9.zip.
GitHub/web yayını ve kurulu VST3 dosyalarının değiştirilmesi yapılmadı.
