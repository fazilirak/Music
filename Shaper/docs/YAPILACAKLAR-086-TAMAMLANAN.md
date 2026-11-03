# Jellybone 0.8.6 — Serbest arayüz boyutlandırma

Fazıl Irak · 03.11.2026

- Köşe tutamacı etkin; host pencere boyutlandırması da desteklenir.
- 1100×960 mantıksal tuval tek content bileşeninde ölçeklenir. Kontrol görünümü ve fare hit-test'i aynı dönüşümü kullanır. Host'un DPI dönüşümü değiştirilmez.
- Minimum 550×480; varsayılan 770×672; maksimum 2200×1920. Serbest en-boyda panel orantılı ve ortalı kalır.
- Boyut processor state'inde tutulur; editör kapanıp açılınca ve DAW state'i geri yüklenince hatırlanır. Efekt parametre kimlikleri ve audio DSP değiştirilmedi.
- Dört CTest grubu geçti: audio_integration, irak_dsp, pitch_dsp, curve_effects.
- Regresyon: minimum/varsayılan/geniş/maksimum geometri; görünür köşe tutamacına JUCE mouseDown/mouseDrag/mouseUp göndererek 770×672 -> 670×572; boyut state roundtrip -> 770×672.
- Görsel kontrol: dist/preview-resizable-minimum.png ve dist/preview-v03-0.png.
- Canlı uygulama açma izni zaman aşımına uğradı. FL Studio açık/erişilebilir değildi ve standart Image-Line kurulum klasörlerinde bulunamadı. FL Studio içinde gerçek host kontrolü henüz yapılmadı.
- Yerel VST3/Standalone ve eşleşen kaynak ZIP'i dist/Jellybone-0.8.6-distribution klasöründe. GitHub/web yayını yapılmadı.
