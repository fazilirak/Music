<!-- Geliştirici: Fazıl Irak | Oluşturma / son düzenleme: 27.10.2026 -->
# 0.2 çoklu efekt makroları

Bu sürüm önceki 17 tek-efekt listesinin yerine geçer. Bütün makrolar aynı ses zincirini farklı oranlar ve eğrilerle sürer. İsim değişikliğine ek olarak ses motoru da yeniden yazılmıştır.

## Intensity nasıl ilerler?

Balanced karakter için filtre 0–100 aralığında logaritmik yükselir; hall yaklaşık %2–82, echo %12–90, phase/frekans kaydırma %18–94, drive %30–100, noise/tonal yükseliş %40–100 aralıklarında büyür. Bunlar bağımsız smoothstep eğrileridir. Karakter, ortak makro eğrisini bükerek bu davranışların erken veya geç gelmesini sağlar.

Sonuç yalnız tek bir efektin dry/wet karışımı değildir: ana gövdenin frekansı, yankı yoğunluğu, feedback, phase hareketi ve katmanların seviyesi aynı anda değişir.

## Modlar

| Mod | Ses hareketi |
|---|---|
| Air Bloom | Bas hafif çekilir, hall açılır. Daha az dramatik bus geçişleri için başlangıç modu. |
| Echo Lift | Bas çekilir, noktalı sekizlik ping-pong echo ile hall birlikte büyür. |
| Skyline | Bas ve orta frekanslar daha ileri süpürülür; phase, echo ve geniş alan yükselişi birleştirir. |
| Heatwave | Orta/üst bölgeye yükselen filtre, tanh drive ve phase ile daha sıcak ve sıkışık doku verir. |
| Spiral Rise | Yukarı frekans kaydırılan katman echo feedback'ine girer; ardışık yankılar yükselir. Çift phase kolu, noise ve ince bir tonal bulut da eklenir. |
| Fracture | Daha kuvvetli drive, yüksek HP, güçlü kaydırılmış echo ve noise. Son bölüm kısmen çözülür. |
| Critical Mass | Bütün katmanlar en geniş aralıkla açılır; son bölümde toplam efekt sesi çekilir. %100 sessizlik, sıfıra dönüş kuru drop verir. |

Bunlar başka üründeki modların ölçülmüş karşılıkları değildir. Herhangi bir referans modla birebir eşleme iddia edilmez.

## Katman ayrıntıları

- HP: iki kutuplu TPT state-variable filtre; hedef frekans örnekleme hızının güvenli bölgesinde tutulur.
- Hall: stereo reverb. 0.2.1 sürümünde çıkışına ayrı HP eklenir: Intensity ile 35 Hz'den en az 1.2 kHz'e, sert makrolarda ana HP son frekansına kadar yükselir. Önceden birikmiş düşük frekanslı kuyruklar da süpürülür. Kuru sinyal reverb modülünün içinde karıştırılmaz.
- Echo: noktalı sekizlik, stereo çapraz feedback. Tempo değişiminde iki gecikme kafası 50 ms çaprazlanır. Echo Lift, 0.2.1 sürümünde daha güçlü ilk tekrar ve daha uzun feedback ile yeniden dengelenmiştir.
- Phase: iki ayrı altı-aşamalı allpass kolu yükselerek süpürülür, sarma anında biri diğerini devralır.
- Frequency shift: 63 tap Hilbert FIR ile tek yan bant kaydırma. Pitch transpozisyonuyla aynı işlem değildir; paralel katman ve feedback içinde çalışır.
- Tonal cloud: sekiz oktavlık aralığa yayılmış sinüslerin yükselmesi ve ağırlıklı devri.
- Noise: stereo gürültünün frekans bandı ve seviyesi makroyla yükselir.
- Drive: telafili tanh. Yalnız üst makrolarda belirgin hale gelir.
- Son fade: Fracture'da kısmi, Critical Mass'ta tam çözülme.

## Dinleme düzeni

Önce kuru dosyayı, sonra Air Bloom, Echo Lift ve Spiral Rise örneklerini karşılaştır. Hepsi aynı stereo techno loop'tur. Sonraki aşama kendi loop'larınla DAW içinde Intensity otomasyonu ve referans eklentiyle seviye eşlenmiş A/B dinlemesidir. Otomatik testler estetik eşleşme sağlamaz.

