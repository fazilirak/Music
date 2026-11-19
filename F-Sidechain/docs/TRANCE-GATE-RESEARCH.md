# Trance Gate — araştırma ve özgün tasarım
Fazıl Irak · 19 Kasım 2026

Bir sesin seviyesini tempo senkronlu açık/kapalı adımlar ve kısa zarflarla biçimlendirme yaklaşımı temel alındı. Başka ürünün preset noktaları veya factory bankası kopyalanmadı. Bunlar evrensel standart isimleri olan sekiz zorunlu desen değil, yaygın ritim ailelerinden türetilmiş özgün seçimlerdir.

## Birincil kaynaklar
- Kilohearts Trance Gate: https://kilohearts.com/products/trance_gate — pad/lead kesme, adım bağlama, step uzunluğu, ADSR, Mix ve sekiz slot iş akışı.
- LennarDigital Sylenth1 kılavuzu: https://www.lennardigital.com/files/Sylenth1Manual.pdf — 4.4.1 Arpeggiator; adım süresi, gate süresi, hold ve velocity aksanları. Bu bölüm bir amplitude gate eklentisi değildir; ritmik artikülasyon yaklaşımı için incelendi.

## Yeni ikinci sıra
1. Classic 16: her onaltılıkta kısa kapı.
2. Offbeat Eighths: sekizlik kontralar, bas/kick aralarında akor açma.
3. Anthem Sync: düzensiz aralıklı onaltılıklar ve aksanlar.
4. Dotted 3-3-2: her yarım barda 3+3+2 senkop ailesi.
5. Triplet Roll: üçleme adımlarında 1/.58/.82 aksan döngüsü.
6. Swing Chops: tek onaltılıkları geciktiren, dönüşümlü aksanlı kesme.
7. Held Chords: uzun tutuşlarla kısa ara kesmelerin karşıtlığı.
8. Final Beat Fill: sekizlik hareketten son vuruşta otuzikilik fill'e geçiş.

Her desen bir bar, Sync 1/1, Mix %100 ve Min/Max 0/1 açılır. X zaman, Y grafik yüksekliğidir; Y=1 sıfır gain demektir. Başlangıç/son noktası sessizdir. Her açılış/kapanış için yalnız gerekli köşe noktaları vardır; örneklenmiş yaylar kullanılmaz. En kısa açılış .002 bar, kapanış .005 bar ile sınırlanır (kısa pulse sürelerinde uyarlanır). Kullanıcı noktaları normal eğri düzenleyicide değiştirebilir, band/preset/session kaydıyla saklayabilir. Eski 0–7 indeksleri ve veri dosyası aynen korunur; yeni indeksler 8–15'tir.

Sayısal reçeteler: TRANCE-GATE-DESIGNS.json. Gerçek DAW'da müzikal beğeni, otomatik süreklilik/tempo testinden ayrı değerlendirilir.
