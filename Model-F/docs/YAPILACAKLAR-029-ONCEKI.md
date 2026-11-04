# Model F — Yapılacaklar

Fazıl Irak · Oluşturulma: 25.10.2026

## Çalışma düzeni

- Kullanıcı “listeye ekle” dediğinde ilgili hata veya istek bu dosyada biriktirilir.
- Her madde için ayrı derleme ve yayın yapılmaz.
- “Yapılacak listesini tamamla” komutu geldiğinde biriken maddeler topluca uygulanır ve uygun testlerle kontrol edilir.
- Tamamlanan maddeler listeden çıkarılır; bitmeyen maddeler gerekçeleriyle korunur. Tümü tamamlandığında liste boşaltılır.
- GitHub ve web yayını, kullanıcının yayın talebine göre yapılır.

## Bekleyen maddeler

- [ ] **Multi-Stage Envelope — seçilen waveform Ctrl + sol tık sırasında Sine'a dönüyor.** (30.10.2026 · Fazıl Irak)
  - Node alanında Sine dışında bir waveform seçilmesine rağmen grafik alanı Ctrl + sol tık ile şekillendirilirken seçim yeniden Sine oluyor. Bu nedenle diğer waveformlar kullanılamıyor.
  - Beklenen: Ctrl + sol tık ile şekillendirme, kullanıcının seçtiği waveformu korumalı ve o waveformu uygulamalı; kendiliğinden Sine'a dönmemeli.
  - Kontrol: Sine dışındaki her waveform seçilip Ctrl + sol tık ile grafik düzenlenerek hem seçimin korunduğu hem ilgili şeklin uygulandığı doğrulanacak.

- [ ] **Ses seviyesi ve gövde — diğer synthesizerlara göre düşük ve cılız duyuluyor.** (02.11.2026 · Fazıl Irak)
  - Kullanıcı gözlemi: Sylenth1, Serum ve UAD Minimoog daha güçlü duyulurken Model F'nin sesi düşük ve cılız kalıyor. Bu gözlem henüz ölçümle doğrulanmadı.
  - Çıkış seviyesini ve tonun gövde/harmonik içeriğini ayrı incele. Osilatör karışımı, filtre/rezonans sırasında bas kaybı, envelope/VCA, polifoni-unison normalizasyonu ve genel kazanç zincirinde gereksiz zayıflama olup olmadığını araştır.
  - Önce aynı MIDI, benzer temel patch, aynı nota/velocity ve efektler kapalı koşullarda ham çıkış seviyelerini ölç; ardından seviye eşlenmiş karşılaştırmayla kalan tonal farkları değerlendir. Farklı fabrika presetlerinin ses yüksekliğini tek başına kalite ölçütü kabul etme.
  - Amaç: müzikal gövdeyi ve kullanılabilir çıkış seviyesini iyileştirmek; yalnızca çıkış gain'ini artırarak ya da kontrolsüz distortion/limiting ile sorunu gizlememek.
  - Kontrol: bass/lead/pad, farklı karakterler, mono/poly ve yoğun akorlarda peak/RMS, headroom, clipping ve CPU etkisini ölç; eski preset/DAW oturumlarının uyumluluğunu ve mevcut seslerinin korunmasını değerlendir. Sesi dinleyerek doğrula.
Global Tuning, kompakt karakter seçimi (Ladder / Edge / Bite / State Variable / Acid / VA Stack) ve CPU incelemesi 26.10.2026 tarihinde Model F 0.2.4 yerel Windows sürümünde tamamlandı.

Kullanım ve sınırlar: docs/KARAKTERLER-024-TR.md. Ölçüm ve test sonuçları: docs/CPU-024-TR.md. Önceki maddelerin arşivi: docs/YAPILACAKLAR-024-TAMAMLANAN.md.

