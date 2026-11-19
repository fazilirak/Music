# F-Kick 0.8.6 — Transient Shaper

Geliştirici: Fazıl Irak · Düzenleme: 03.11.2026

## Kullanım

Sağ sütunda Tempo'nun altındaki üç küçük knob, stereo toplamın vuruş/zarf yapısını düzenler. Pencere 912 × 511, ana grafik 570 × 244 mantıksal piksel olarak korunur.

- **Attack (−100…+100%)**: İlk vuruşu kısar veya güçlendirir. Uç değerler −12/+12 dB kazanca karşılık gelir.
- **Pump (0…100%)**: İlk 8 ms'yi yükseltmeden bırakır; sonrasına yumuşak bir kısılma uygular. %100 en güçlü bölgede −18 dB'dir. Sustain ve Attack sıfırken bu kontrol hiçbir örneğe birden büyük kazanç uygulamaz.
- **Sustain (−100…+100%)**: Devamı ve kuyruğu kısar veya güçlendirir; uç değerler −12/+12 dB'dir.

Üçü sıfırken transient işlemi örnekleri değiştirmeden döndürür. Knoblara çift tıklamak sıfıra döndürür. Değeri alttaki metin alanından yazabilir, sürükleyebilir veya tekerlekle değiştirebilirsiniz. Bu ayarlar preset/DAW oturumuyla kaydedilir ve otomasyona açıktır. Artı Attack/Sustain daha yüksek çıkış üretebilir; gerekirse mevcut Output ile seviye bırakın.

## Ses yolu ve zamanlama

Kuru kick/sample, mevcut filtre/drive ve FX/Bass EQ yolundan geçer. Transient Shaper bundan sonra stereo toplama, DC süzgeci ve mevcut Output/tepe sınırlamasından önce uygulanır; FX kuyrukları da Sustain etkisine dahildir. İki kanal tek kazançla işlenir, sağ/sol dengesi ayrı ayrı değişmez. Yeni gecikme/lookahead eklenmez.

MIDI note-on, vuruş zamanını doğrudan bildirir. Geç başlayan sample'lar için kuru kick/sample üzerinden hızlı/yavaş zarf farkı ayrıca başlangıç algılar; reverb/echo dönüşleri bu algılayıcıyı kendi başına yeniden tetiklemez. Bu, F-Kick için MIDI bilgisiyle çalışan üç bölgeli bir kazanç zarfıdır; kullanıcıya eşik veya ayrı attack-time/release-time ayarı sunulmaz.

- Attack bölgesi: İlk 2 ms tam etki; 24 ms'ye kadar yumuşak sönüş.
- Pump bölgesi: 8–20 ms yumuşak giriş, 20–45 ms derin bölüm, 100 ms'ye kadar yumuşak çıkış.
- Sustain: 35–90 ms arası yumuşak giriş, devamında kuyruk kontrolü.
- Kontrol kazançları 6 ms, ortak kazanç zarfı 0,4 ms yumuşatılır. Üç hedef de tam sıfıra geldiğinde doğrudan ve tam bypass önceliklidir.

Süreler örnek sayısına değil saniyeye bağlıdır. Fonksiyon callback içinde tahsis, kilit, dosya erişimi, GUI çağrısı veya örnek başına üstel kazanç hesabı yapmaz.

## Bakım haritası ve uyumluluk

- `Source/TransientShaper.h`: Algılama, zaman bölgeleri, kazançlar ve sıfırda doğrudan dönüş.
- `Source/Engine.cpp`: MIDI tetikleme ve son stereo ses yoluna bağlantı.
- `Source/PluginProcessor.cpp`: `tsAttack`, `tsPump`, `tsSustain` parametreleri; önceki 155 kimlik/indeks aynen kalır, yeni üçü sonda.
- `Source/TransientKnob.h`: Kullanıcının istediği üç kompakt rotary kontrol; bu ekleme eski sayısal kontrol şeritlerini değiştirmez.
- `Tests/TransientTests.cpp`: Bypass, bölgesel ses etkisi, stereo bağlantı, geç sample, farklı örnek hızları, state/eski preset, blok boyutu ve GUI kontrolleri.

FKICK_PRESET sürüm 1 korunur. Eski presetlerde eksik üç yeni parametre varsayılan sıfırdan tamamlanır. 128 fabrika preseti bu işlemi sıfırda açar; daha önce kaydedilmiş sesler kendiliğinden farklılaştırılmaz. Bu sürüm yerel Windows incelemesi için hazırlanır; ayrıca yayın istenmeden GitHub/web/e-posta güncellemesi yapılmaz.
