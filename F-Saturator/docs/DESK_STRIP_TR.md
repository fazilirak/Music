<!-- Geliştirici: Fazıl Irak | Son düzenleme: 26.10.2026 -->
# F-Saturator 0.5.1 — Dinamik ve ton şeridi

Model düğmelerinin SOLUNDA Transient ve Comp, SAĞINDA Bass ve Treble bulunur.
Bu kontroller Console, Transformer, Tape, Triode ve Pentode modellerinin hepsinde çalışır.
İşlevsel referans SDRR DESK kontrol gruplarıdır; algoritma ve çizimler özgündür.
Klanghelm kodu/kaplaması kullanılmaz ve birebir aynı ses veya devre eşdeğerliği iddiası yoktur.

## Kullanım
- **Transient (-100..+100%)**: Soft yönünde atakları yumuşatır, Sharp yönünde öne çıkarır. Model sürüşünden önce stereo bağlı hızlı/yavaş zarf farkıyla çalışır. 0% nötrdür.
- **Comp (0..100%)**: Stereo bağlı, soft-knee feedback kompresör. Miktarla oran 1:1..6:1, eşik -12..-30 dBFS değişir; knee 8 dB'dir. 0% kapalıdır. Otomatik makeup yoktur: ses yüksekliğini mevcut Output ile karşılaştırın.
- **Fast ATT**: Kompresör attack süresi normal 22 ms, açıkken 1.5 ms. Atakları daha hızlı yakalar.
- **Fast REL**: Kompresör release süresi normal 220 ms, açıkken 55 ms. Gain reduction daha hızlı bırakılır. Çok hızlı ayarlar düşük frekanslarda duyulur modülasyon üretebilir.
- **Bass (-12..+12 dB)**: 120 Hz geniş low shelf.
- **Bass Tight**: 32 Hz sub-bass filtresini miktara bağlı harmanlar; pozitif Bass ayarında 290 Hz civarında geniş bir azaltma ekler. Amaç gövdeyi artırırken sub/low-mid yığılmasını kontrol etmektir.
- **Treble (-12..+12 dB)**: Normalde 6 kHz geniş high shelf.
- **Treble Tight**: Geçişi 3.5 kHz'e kaydırır; negatif Treble ayarında 2.3 kHz civarında hafif presence desteği ekler.

Tight tek başına, ilgili EQ 0 dB iken sesi değiştirmez. Transient=0, Comp=0 ve iki EQ=0 dB eski sinyal yolunu korur. Düğmeleri dikey sürükleyin; sayısal alanı düzenleyebilir veya çift tıklayarak sıfıra dönebilirsiniz. Alt şerit için pencere 1040×640 sabittir (0.5.2); elle boyut değiştirme kapalıdır.

## Sinyal yolu / geliştirme
Giriş → Transient → mevcut 8× saturation/filtre yolu → host hızına dönüş → Auto Gain → Comp → Bass/Treble → Stereo Width → Mix/Output → hizalanmış Bypass.
Yeni işlemler host hızında çalışır; ek lookahead ve bildirilmesi gereken ek gecikme yoktur. Mix=0 kuru yolu korur. Ana Auto Gain yalnız eski saturation telafisidir; yeni kompresör ve EQ'yu loudness-normalize etmez.

Source/ChannelStripDSP.h: dedektörler, feedback, rampalar, sabit bellekli biquad'lar. Filtre katsayıları değişiklik varsa 16 host örnekte bir yenilenir. Ses callback'inde yeni dosya erişimi, kilit veya dinamik bellek ayırma yoktur.
Source/DeskKnob.h: rotary slider, sayısal giriş ve APVTS SliderAttachment. Source/PluginEditor.cpp: yerleşim, anahtar attachment'ları ve çizim.

Yeni kimlikler eski 16 parametrenin sonuna eklenir: transient, comp, bass, treble, fast_attack, fast_release, bass_tight, treble_tight. State sürümü 3'tür; eski dosyada eksik alanlar nötr eklenir. Fabrika presetleri yeni alanları sıfırlar; eski preset sesleri değiştirilmez.

Tests/ChannelStripChecks.h nötr bit doğruluğu, shelf kazancı, iki Tight etkisi, Soft/Sharp, kompresyon, Fast ATT/REL, stereo bağ, 44.1/48/96/192 kHz kararlılık ve sessiz kuyruk kontrollerini içerir. DSPTests.cpp otomasyon sırası, state recall, legacy migration, GUI sınırları ve etkin düğme görüntüsünü doğrular. Sentetik testler müzikal dinleme ve bütün DAW host testlerinin yerine geçmez.

## 0.5.1 kompakt yerleşim
Ana pencerenin eski 1040×640 boyutu geri getirildi. Yeni knob çapları küçültüldü; Fast ATT/REL ve Tight anahtarları 7 px LED göstergelere dönüştürüldü. ATT/REL etiketlerinin üzerine gelerek tam işlev açıklamasını görebilirsiniz. Ses motoru ve parametre sözleşmesi değiştirilmedi.
