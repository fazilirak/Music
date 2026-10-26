# Model F 0.2.6 — Rezonans göstergesi ve Acid düzeltmesi

Fazıl Irak · 26.10.2026

## Turuncu alan ne demek?

Emphasis çevresindeki ince yay yüksek rezonans bölgesini gösterir. Ses seviyesi ve dar frekans tepesinin belirginleşebileceğini hatırlatır; limiter, otomatik koruma veya fiziksel güvenlik garantisi değildir. Cutoff, çalınan nota, Overload ve sesin giriş seviyesi sonucu değiştirir. Çıkış seviyesini düşük tutarak ayarlayın.

| Karakter | Turuncu başlangıcı (0–10) | Gerekçe |
| --- | ---: | --- |
| Ladder | 7.5 | Mevcut fabrika kalibrasyonunun rejenerasyon bölgesi; özgün DSP değiştirilmedi. |
| Edge | 8.3 | Low-pass Q yaklaşık 2.5 ve üzerine yaklaşır. |
| Bite | 8.2 | İki rezonans katından low-pass önce yüksek Q bölgesine girer; Shape HP davranışını da etkiler. |
| State Variable | 9.0 | Rezonanslı çıkışlarda Q yaklaşık 2.5 bölgesi; seçili çıkış karışımı sonucu etkiler. |
| Acid | 8.0 | Düzeltilmiş üst rezonans bölgesi; eski 7.7 civarındaki erken kendiliğinden salınım kaldırıldı. |
| VA Stack | 9.4 | Rezonanslı ilk iki kutuplu katta Q yaklaşık 2.5; Shape 12/24 karışımıdır. |

Bu başlangıçlar görsel rehberdir; her ses için ayrı ölçülmüş bir tehlike eşiği değildir. Panel 1120×760, Emphasis yarıçapı 27 piksel olarak korunur; yay ve turuncu ölçek çizgileri seçilen karakteri izler.

## Acid'de ne düzeldi?

Önceki filtre her kademenin içine (1 + 0.09*r) küçük-sinyal kazancı koyuyordu. Dört kademede bu etki çarpılıyor, analitik geri besleme paydasının varsaydığı birim eğimle uyuşmuyordu. 7.7–7.8 Emphasis aralığında erken salınıma ve ani seviye sıçramasına yol açabiliyordu.

Ek kademe kazancı kaldırıldı. Maksimum geri besleme 3.95 olarak korunur; ideal dört kutuplu kritik değer 4'tür. Üst çeyrekte sürekli değerli ve sürekli eğimli bir ayar eğrisi maksimuma daha yumuşak yaklaşır. Canlı rezonans değişimi 20 ms zaman sabitiyle süzülür; ilk nota mevcut parametre değerinden başlar. Bu, sert bir çıkış limiteri eklemek değildir.

Acid'de sürükleme 2.5 kat hassastır: normal 0.02 Emphasis/piksel; Shift 0.002/piksel, saklanan parametre adımı yine 0.01'dir. Tekerlek normal adımı 0.05, Shift adımı 0.01'dir. Diğer karakterlerin fare hızı ve DSP davranışı korunur. Eski preset/otomasyon parametre kimlikleri ve sayısal değerleri değişmez; Acid'in filtre cevabı bu düzeltmeyle farklı duyulabilir.

## Ölçüm

Doğrudan filtre testi; 48 kHz, 100/500/2000 Hz sinüs, 1e-9 A küçük-sinyal girişi ve 0.00012 S bus iletkenliği, Shape=0. Her konumda 0.5 saniye üretildi, son 0.25 saniyenin RMS'i ölçüldü. Emphasis 0–10 arasında 0.1 adımlarla tarandı.

| Cutoff | Önce: en büyük komşu adım artışı | Sonra |
| --- | ---: | ---: |
| 100 Hz | 7.95 dB | 1.80 dB |
| 500 Hz | 25.74 dB | 2.05 dB |
| 2000 Hz | 45.57 dB | 2.04 dB |

Bu sayı ses sistemindeki mutlak çıkış seviyesini belirtmez; belirli küçük-sinyal ölçümündeki komşu knob adımları arasındaki farktır. Diğer dört alternatif karakterin aynı taramadaki 1212 satırı bit/sayısal olarak aynıdır. Ladder yolu bu modülde olmadığı için değiştirilmedi. Ek regresyon testleri 8/44.1/48/96/192 kHz'de sonlu çıkış, 20 ms yumuşatma, reset ve uyarım sonrası sönümü denetler.

## Kaynak haritası

- `crates/model-f-core/src/character_filter.rs`: geri besleme tutarlılığı, üst çeyrek eğrisi, Acid rezonans yumuşatması ve görsel eşikler.
- `crates/model-f/src/panel.rs`: turuncu yay/ölçek ve karaktere göre sürükleme/tekerlek adımları.
- `crates/model-f/src/editor.rs`: karakter bilgisini ortak fare yoluna verir.
- `crates/model-f/examples/preview.rs`: `acid-resonance.png` sahnesi.

Yeni audio callback tahsisi veya ek örnekleme katmanı yoktur. Gerçek DAW, preset ve dinleme doğrulaması otomatik testlerden ayrıdır.
