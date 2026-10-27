# F-Kick yapılacaklar

## Tamamlanan geliştirmeler — 0.8.0

- [x] Room Mix altında HP/LP yan yana.
- [x] Alt satıra sayısal Bass Lift/Q: F-EQ ile aynı 300 Hz shelf davranışı.
- [x] EQ toplam kick/sample/FX çıkışında; 0 dB eski sesi korur.
- [x] Yeni alanları sona ekleme, state ve eski preset uyumluluğu.
- [x] Frekans/routing/otomasyon/UI kontrolleri.

## Tamamlanan geliştirmeler — 0.7.0

- [x] Room düğmesi yerine sayısal 0–100% Room Mix; eski On=%100 seviyesi korunur.
- [x] Room Mix / FX HP / FX LP sample ekranının sağında alt alta.
- [x] Grafik yüksekliği 126→178 px; gerçek çizim alanı da daha yüksek.
- [x] Sample sayıları içeride tek satır; üst katman/hit-test erişimi korunur.
- [x] SC dışında LFO/AD UI ve menü kaldırıldı; eski preset sesleri DSP'de korunur.
- [x] Room ara değerleri, eski %100 ses eşitliği, kolon ve hit-test kontrolleri.
## Tamamlanan geliştirmeler — 0.6.0

- [x] Ana Mod sayfasını sade SC ile değiştirme: Pump / Smooth / Late / Gate, yatay grafik sürükleme, Shift, Mix.
- [x] Yalnız rumble/echo/room dönüşlerini şekillendirme; kuru kick/sample korunur.
- [x] Host PPQ'ya kilitli 1/4 döngü; transport yokken MIDI retrigger ve mevcut Tempo.
- [x] FX'te tek Room düğmesi ve ortak efekt dönüşü için FX HP / FX LP metinleri.
- [x] Eski LFO/AD ve mapping'leri Advanced modulation menüsünde koruma.
- [x] Yeni altı parametreyi sona ekleme; 0.5 state/golden WAV bypass uyumluluğu.
- [x] Dört SC ses örneği, gerçek GUI snapshot'ları ve otomatik testler.

Otomatik doğrulama ve görsel inceleme: 27.10.2026. Ableton'da canlı dinleme geri bildirimi bekleniyor.
## Tamamlanan geliştirmeler — 0.5.0

- [x] Ableton / C3 / 069 Rumble - Basement geri bildirimi: kısa kalan kuru gövdeyi doldurma,
  hızlı amplitude başlangıcı ve kuru kick'i öne alan efekt dönüşü dengesi.
- [x] Sound / Punch: faz kilitli kısa harmonik ve gövde yoğunluğu; sample/sampler yolu korunur.
- [x] FX / Rumble / Gain: bağımsız wet dönüş trim'i, −24…+24 dB, 20 ms rampa.
- [x] 128 presetin yeni motorla seviye/headroom/kararlılık kontrolü; 64/48/16 dağılımı korundu.
- [x] Techno rumble seviyelerinin kuru vuruşa göre ölçülüp ayrı dengelenmesi.
- [x] C3/069 eski-yeni, eşit peak kuru ve efektli ses dosyaları; kuru 20–150 ms RMS +10.70 dB.
- [x] Önceki 121 parametrenin ID/sırasını koruma; eski presetlerde iki yeni alanı nötr başlatma.
  Legacy Deep House state'i önceki WAV'ı byte-byte aynı üretiyor.
- [x] Yeni kick'lerde Track tail varsayılan kapalı; MIDI36/60 aynı kökü korur. Sampler nota takibi sürer.

Otomatik test / gerçek GUI snapshot incelemesi: 27.10.2026. C3/069 canlı Ableton dinleme geri bildirimi bekleniyor.

## Tamamlanan geliştirmeler — 0.4.0

- [x] **128 kick: 64 General, 48 Techno, 16 Experimental.** Son kullanıcı dağılımı uygulanmıştır.
  Genel grupta yaygın, kontrollü kick yapıları; techno grubunda rumble/hard/acid/echo/industrial/
  hypnotic/minimal/schranz tarifleri; deneysel grupta farklı konturlar ve modülasyonlar bulunur.
  Kaynak araştırması ve tam liste `PRESET_KATALOGU_TR.md` içindedir. Ticari sample/preset kopyalanmadı.
  Eski techno sesleri 065–068'e taşındı. Sampler Init araç menüsündedir; eski state parametreleri korunur.
  Banka sayımı, tüm menü seçimleri/oklar, 128 taşınabilir preset, örnekleme hızları, seviye/DC/kuyruk
  testleri geçti. Teknik ölçümler algısal beğeni veya her mix için sonuç garantisi değildir.
- [x] **HP knob'u Cutoff gibi metin kontrolüne dönüştürüldü.** Drag, wheel, çift tık giriş,
  `1.2k` ve `Off` desteklenir. Bağımsız HP ve parametre ID'si korundu; rotary kontrol kalmadı.
- [x] **Saw Up / Saw Down ayrı ve menüde yan yana.** Önizlemeleri yükselen/düşen yönleri gösterir.
  Eski Saw fiziksel shape=2 olarak korundu; Saw Down sona shape=5 eklendi. Menü görsel sırası
  parametre indekslerine açıkça eşlenir. Yön, menü, state ve GUI testleri geçti.
- [x] **Windows sürükle-bırak aktarım yolu yeniden uygulandı.** Editörün kendi JUCE peer HWND'sinde
  OLE kayıt ve Unicode CF_HDROP aktarımı doğrudan sample yükleyicisine bağlandı. Host ebeveyn
  penceresine dokunulmaz. Root editör de ses dosyası kabul eder; sample alanı ve Load sample yazısı
  aynı asenkron yolu kullanır. Gerçek OLE IDataObject ve Drop callback testi geçti.
  **Canlı Explorer→Standalone/Ableton doğrulaması henüz tamamlanmadı; aşağıda açık tutuluyor.**

Uygulama / otomatik test / görsel inceleme: 27.10.2026. Kullanıcı dinleme ve kullanım geri bildirimi bekleniyor.

## Tamamlanan geliştirmeler — 0.3.0

- [x] Filter panelinde mevcut low-pass ile birlikte çalışan ayrı HP knob'u (Off–2 kHz).
- [x] Stereo rumble; decay, bağımsız HP/LP, Drive, Duck ve Recover.
- [x] Tempo senkronlu / serbest echo; Kick/Rumble kaynak, feedback ve LP.
- [x] Soft / Clip / Fold distorsiyon karakterleri.
- [x] Senkron / Hz LFO, beş dalga şekli, faz ve MIDI retrigger.
- [x] Delay / Attack / Decay / Curve tek atımlı modülasyon zarfı.
- [x] On hedefte eşzamanlı LFO/AD bağlantıları, signed derinlik, sağ tık atama ve göstergeler.
- [x] 760 × 360 içinde Sound / FX / Mod görünümleri; kaynak önizlemeleri ve tempo alanı.
- [x] Dört özgün techno preset, dört 135 BPM ses örneği, efekt kuyruğunu içeren WAV export.
- [x] İlk 74 parametrenin sırasını koruma; eski presetlerde yeni efektleri nötr varsayılana taşıma.

Geliştirme, otomatik test ve görsel inceleme: 27.10.2026. Kullanıcı dinleme ve kullanım geri bildirimi bekleniyor.

## Tamamlanan geliştirmeler — 0.2.1

- [x] **Sample alanında Start–End dışındaki bölgeleri hafif karartma.**
  Start işaretinden önceki ve End işaretinden sonraki dalga bölgelerine hafif karartma
  uygulanmalı; seçilen çalma alanı normal parlaklığında kalmalı. Böylece çalınacak bölüm
  daha belirgin görünmeli ve karartılan bölgelerdeki dalga şekli hâlâ seçilebilmeli.
  İstek / tamamlanma tarihi: 27.10.2026. Dış bölgeler %42 saydam siyahla karartılır;
  seçili alan ve işaretçiler parlak kalır. Piksel karşılaştırması ve görsel inceleme yapıldı.

- [x] **Windows Dosya Gezgini'nden sample alanına sürükle-bırak — ilk uygulama (0.2.1).**
  Kullanıcı gerçek kullanımda hata bildirdi; 0.4.0 native OLE köprüsüyle yeniden uygulandı.
  Aşağıdaki bilgiler ilk uygulamanın otomatik test kapsamıdır; canlı doğrulama aşağıda açık kalır.
  Kullanıcı bir ses dosyasını Windows Explorer penceresinden F-Kick'in sample/dalga alanının
  üzerine bıraktığında dosya yüklenmeli ve dalga görünümü güncellenmeli. Mevcut dosya bırakma
  panel, kontrollerinin ebeveyni olan açık bir native dosya bırakma hedefi yapıldı. Hedef
  sürükleme sırasında vurgulanır; tek dosya arka planda yüklenir ve Sample görünümü açılır.
  Waveform/Label üzerinden native ComponentPeer hedef araması ve drop teslimi, Unicode dosya
  yolu, hatalı dosya reddi ve derlenmiş VST3'te yüklenen sample'ın state/ses yoluyla korunması
  otomatik testlerden geçti. İstek / geliştirme tamamlanma tarihi: 27.10.2026.

## Bekleyen canlı doğrulama

- [ ] Gerçek Windows Dosya Gezgini'nden Standalone'a ve Ableton VST3'e fareyle sample bırakma;
  hem waveform hem Load sample yazısının üzerine. 0.4.0 için bilgisayar kullanım aracıyla
  pencere seçimi/etkinleştirme yeniden denendi, `failed to activate captured window` hatası
  tekrarladı. Bu yüzden kullanıcıda bildirilen hatanın canlı çözümü doğrulanmış sayılmadı.
  Native kayıt/CF_HDROP, JUCE hedef yolu, asenkron sample yükleme ve derlenmiş VST3 testleri geçti.


