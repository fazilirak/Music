# F-Kick 0.8.4 doğrulama

Fazıl Irak · 03.11.2026 · Windows x64 / MSVC / JUCE 8 / Release

VST3, Standalone ve FKickTests başarıyla derlendi. Son CTest 1/1 geçti; ayrıntılar CTest-084.log. Bu test birden fazla DSP, preset, state, dosya-drop, gerçek VST3 instantiate/editor ve GUI kontrolünü çalıştırır.

## Yeni kontroller

- Gerçek 0.8.3 TechnoFX kaynaklarıyla 44.1/48/96 kHz × 6 varyant: 18 Classic karşılaştırması, en büyük örnek farkı **0**. Eski 128 factory preset ve önceki state testleri de geçer.
- Split/Groove dry koruması; Room SC değişiminin yalnız rumble sinyaline karışmaması, Rumble SC'nin kendi dönüşünde duyulur fark üretmesi.
- Yüksek drive/gain altında stereo-linked rumble limiter -18 dBFS dönüş tepe sınırı. Bu toplam çıkış/true-peak garantisi değildir.
- Hybrid motorunun kick/sample/rumble/echo/room kapalıyken ses üretmesi ve bass-only L/R eşitliği. MIDI 29 frekansı ~43.656 Hz; MIDI 41 ile oktav oranı doğrulandı.
- Groove/Hybrid için 90 → 170 BPM sürekli PPQ geçişi; 127/512 blok eşitliği ve 257 blokla oturum geri çağırma. Panic sonrası bass yeniden başlamaz.
- 12 özgün ek reçete, 44.1/96 kHz sonlu çıkış ve 48 kHz örnekler; sessizlik/kırpılma/DC sınırları. .fkick dosyaları ve 24-bit stereo WAV'lar Rumble-Recipes içindedir.
- İngilizce gerçek editör snapshot'ları: Classic/Groove/Hybrid, Routes ve Bass. 912 × 432 sınırı; Limit ve MIDI root düğmelerinin parent/görünürlüğü. Son PNG'ler ayrıca gözle kontrol edildi.
- Kaynak incelemesi: yeni sample işleme yolunda dinamik tahsis, dosya I/O, bekleyen kilit veya GUI çağrısı eklenmedi. Parametrelerden gain dönüşümleri begin() içine alındı; bass perde/kesim katsayıları değişiklikte hesaplanıyor. Yeni lookahead/raporlanan plugin gecikmesi yoktur.

## Ölçülen DSP maliyeti

48 kHz, 1 saniye stereo ses, 3 çalışmanın ortanca süreleri. Yalnız TechnoFX motorunun çevrimdışı mikro ölçümüdür; DAW CPU yüzdesi veya gerçek zaman garantisi değildir. Sistem yükü küçük farkları etkiler.

| Yol | 1 saniye sesin hesap süresi |
|---|---:|
| 0.8.3 referans | 29.334 ms |
| 0.8.4 Classic | 30.424 ms |
| 0.8.4 Groove | 34.151 ms |
| 0.8.4 Hybrid | 14.847 ms |

12 reçetenin 48 kHz maksimum sample peak aralığı 0.203035–0.303958; ayrıntılar RUMBLE-084-METRICS.csv. Bunlar sabit test MIDI/ayar koşullarıdır; her kullanıcı ayarı için headroom taahhüdü değildir.

## Açık doğrulama

Ses dosyaları üretildi fakat burada kulakla dinleme yapılmadı. Kullanıcının Ableton/FL Studio oturumunda dinleme, mono faz, çoklu instance yükü ve aktif otomasyon denemesi yapılmalı. Gerçek DAW kullanımını veya macOS/Linux derlemesini tamamlandı olarak sunmuyoruz. Yerel Windows paketi hazırlanır; GitHub/web yayını bu görev kapsamında yapılmaz.

## Önceki doğrulama kayıtları

# F-Kick 0.8.0 doğrulama

Fazıl Irak · 27.10.2026 · Windows x64 / MSVC 19.51 / JUCE 8 / Release

VST3, Standalone ve FKickTests derlendi. CTest 1/1 geçti; test gövdesi 23.01 saniye. Tam çıktı CTest.log içindedir.

## 0.8.0 Bass EQ

Toplam kick + sample + rumble/echo/room çıkışında F-EQ ile aynı 300 Hz low shelf: ±12 dB, Q 0.1–2.0 (varsayılan .707). HP/LP yalnız FX dönüşüne uygulanmaya devam eder. Output/peak koruması EQ sonrasındadır.

- 44.1/48/96/192 kHz, gain ±12 dB ve Q .1/.707/2 uçlarında 300 Hz merkez tepkisi yarı dB (tolerans .02 dB), 5 Hz / 10 kHz gerçek F-EQ tepkisine eşleşme (tolerans .02 dB).
- Q değişiminin 500 Hz geçiş tepkisine etkisi, stereo ayrılığı ve sonlu ses çıktısı.
- 0 dB bit-exact bypass; gain/Q otomasyonunda yumuşatma.
- Toplam dry+FX çıkışını dışarıdan aynı EQ ile işleyerek iç routing ile örnek düzeyinde eşleşme.
- EQ etkin state roundtrip ve 127/512 blok eşitliği.
- Yeni bassLift/bassQ host parametreleri sona 129/130 indekslerinde eklendi; ilk 129 alan korunur, toplam 131.
- Eski state'te gain 0 / Q .707; önceki golden WAV sesleri aynı.
- Gerçek UI metin editörlerinde +6 dB ve 1.20 girişi, HP/LP ve Bass Lift/Q yatay satırları.

## Korunan kontroller

Room artık 0–100% sayısal Mix kontrolüdür. Sağ sütun Room Mix, altında yan yana HP/LP, onun altında Bass Lift/Q ve en altta Tempo içerir. Grafik yüksekliği 126 pikselden 178 piksele, iç çizim yüksekliği 94 pikselden 116 piksele çıktı. Sample Gain/Tune/Pan/Start/End değerleri grafik içinde tek satırdır. Dış pencere 760×360 olarak korundu.

Ek LFO/AD düzenleyicisi ve menü erişimi kaldırıldı; yalnız sade SC arayüzü kaldı. Eski presetlerin sesini koruyan LFO/AD DSP ve state alanları motorda saklanıyor.

| Doğrulama | Sonuç |
|---|---|
| Room %50 / %100 wet RMS oranı | 0.5; tolerans 0.001 içinde |
| Room %100 ile önceki 0.6 Room On WAV karşılaştırması | Byte-byte aynı |
| Room ID ve host sırası | room / index 126; ilk 129 parametre korundu |
| Room parametre türü | Sürekli AudioParameterFloat; 0/1 eski uçları korundu |
| Room 0.37 state save/restore, aynı ses | Geçti |
| Room Mix alanına 50%, HP 65 ve LP 5k yazma | Geçti |
| Room altında HP/LP yatay sıra ve grafikle çakışmama | Geçti |
| Büyüyen gerçek çizim alanı | Geçti |
| Beş inline sample sayısında gerçek component hit-test | Geçti |
| Ek modulation düzenleyicisinin bulunmaması | Geçti |
| Pump/Smooth/Late/Gate, yatay drag, reset, Mix/Shift | Geçti |
| SC ve return HP/LP ile kuru kick farkı | Tam 0 |
| Room ilk reflection öncesi kuru kopya sızmaması ve duyulur kuyruk | Geçti |
| Host PPQ, negatif PPQ/seek, MIDI fallback | Geçti |
| SC/Room/filtrelerle 127/512 örnek blok eşitliği | Fark < 2e-6 |
| 0.5 bypass state ve golden WAV uyumluluğu | Byte-byte aynı |
| 128 preset, 64 General / 48 Techno / 16 Experimental | Geçti |
| Preset seçimi ve önceki/sonraki gezinme | Geçti |

MIDI offset/velocity, sustain/note-off/CC64, loop/reverse, voice stealing/panic, sample sahipliği/gömülü sample, Undo/Redo, mono bus, hatalı preset reddi, legacy LFO/AD, HP/rumble/echo/duck, Drive uçları, çoklu sample-rate/headroom/DC/tail kontrolleri tekrar geçti. Windows OLE/CF_HDROP/native callback ve derlenmiş VST3 scan/MIDI/audio/state/editor kontrolleri de geçti.

069 Basement equal-peak kuru 20–150 ms RMS kazanımı +10.70 dB olarak korundu. Bu sürümde Bass Lift varsayılan 0 dB olduğu için fabrika preset sesleri korunur.

## Görsel ve ses kanıtı

F-Kick-main.png, F-Kick-bass-eq.png, F-Kick-room-filter.png ve F-Kick-sidechain.png gerçek JUCE editöründen alınan 760×360 görüntülerdir; görsel olarak incelendi. Sayısal alanlar, sağ sütun, inline sample satırı ve genişleyen grafik okunuyor; örtüşme/kırpılma görülmedi.

F-Kick-SC-1.wav…F-Kick-SC-4.wav dört SC şeklinin ses örnekleridir: 069, C3/MIDI60, velocity110, 120 BPM, Room %100 ve FX filtreleri. Dört vuruş ve kuyruk; 48 kHz stereo 24-bit. FX_RACK_METRICS.txt filtre ölçümlerini içerir. Önceki kick A/B ve kategori seçkileri yeniden üretildi.

## Doğrulama sınırları

Canlı Ableton dinleme ve Explorer→Standalone/Ableton fiziksel fare sürüklemesi bu turda yapılmadı; otomatik host/native testleri canlı kullanım testi olarak sunulmaz. macOS derlenmedi.

SC harici audio sidechain algılayıcısı değildir; sabit 1/4 nota volume eğrisidir. Room karakteri sabittir, miktarı Mix ile ayarlanır. Eski LFO/AD preset hareketleri DSP içinde sürer. Drive/Punch oversampling ve peak korumasında lookahead bulunmaz.


## 0.8.2 — 28.10.2026

Release Windows x64 derleme, Standalone/VST3 ve CTest geçti. 112 temiz preset: zorunlu saturasyon ve gizli Drive yönlendirmesi yok; ölçülmüş peak 0.5. Temiz 100 Hz sinüs harmonik oranı -137.35 dB; %65 Punch kontrol karşılaştırması -13.52 dB. Eski fixture WAV'ları Windows'ta byte-byte eşit; 0.8.1 kaydedilmiş Room state'i ile eski Room sesi de birebir aynı. 128 preset state/GUI/headroom/DC/kuyruk/multi-rate, gerçek VST3 scan/instantiate/MIDI/audio/state/editor ve native dosya sürükleme testleri geçti. Gerçek DAW/aygıt ve öznel dinleme ayrı kontrollerdir.
