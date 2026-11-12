# Jellybone 0.9.1 — Yeniden tasarlanan rack bankası
Fazıl Irak · 13.11.2026

160 rack preseti, 10 kategoride aynı isim/numara düzeni korunarak yeniden oluşturuldu.
Önceki fabrika dosyalarının yedeği: dist/backups/Factory-bank-before-redesign-2026-11-13.zip.
Kullanıcının kaydettiği rack'ler ve DAW oturumları değiştirilmez. Yeni eğrileri duymak için
rack presetini yeniden seçin. Eski oturum kendi kaydedilmiş eğrisini açmaya devam eder.

## Tasarım yaklaşımı
Cableguys'ın resmi ShaperBox 3 kullanım kılavuzundaki Arc / S-Curve ve müzikal grid
çalışma yaklaşımı incelendi:
https://downloads.cableguys.com/Cableguys-ShaperBox-3-Manual.pdf (s.37–40).
Özgün preset verileri, bir başka ürünün dosyaları içe aktarılmadan tasarlandı.

Eski sabit 32/64/96 adımlı grafik örneklemesi kaldırıldı. Bir yay 33–97 düğümle taklit edilmez.
Curve, S-curve, HalfSine ve segment tension ile gerçek eğri çizilir. Ek düğümler yalnızca
bekletme, vuruş, vurgu, susma veya kasıtlı yön değişimini tanımlar. Düz sekizlik ritimde
birkaç nokta yeterliyken, asimetrik sekiz vuruşun ayrı attack/hold/release bölümleri daha fazla
nokta gerektirir; bu bir yayın küçük parçalara bölünmesi değildir.

## Kategoriler
| Kategori | Tasarım amacı |
|---|---|
| Ducking | Kick'e gerçek boşluk açma, erken/geç toparlanma, uzun sub, vokal alanı, offbeat ve broken pump |
| Rhythm | Tam kapanan trance/chop, sekizlik ve onaltılık, swing/üçleme, eşit olmayan vurgular, çift vuruşlar |
| Filter Motion | Rezonant pluck, iki aşamalı açılma, vowel benzeri geçiş, acid aksan, yavaş süpürme |
| Stereo | Merkezde başlayan drift, sağ/sol vurgu, sekizlik bounce, üçlü orbit ve merkeze dönüş |
| Time Motion | Amaca uygun kırılma noktalarıyla half-time, repeater, tape pull ve backspin |
| Space | Ayrı Wet/Send eğrileriyle nefes alan oda, gated hall, reverse bloom ve kararan kuyruk |
| Multiband | Bası tutan, orta/üst bantlarda bağımsız hareket eden farklı hız ve derinlikler |
| Creative Racks | Birbirine cevap veren Volume/Filter/Pan/Time/Space; seçili rack'lerde Width ve Delay |
| Saturation Motion | Drive modeline göre vurgu, bekletme, ısı yükselişi ve manyetik hareket |
| Pitch Motion | Gerçek nota basamakları, kontrollü glide, vibrato, wow ve ayrı formant hareketi |

## Öne çıkan değişiklikler
- Clean Quarter'da başlangıç sıfır, kısa bekletme, tek gerilimli toparlanma ve döngü sonu yumuşak dönüş.
- Long Kick / Sub Recovery'de daha uzun temiz alan; hızlı ve uzun release birbirinden ayrılır.
- Broken Pump tekdüze kopyalama yerine ikinci, daha küçük ve gecikmiş bir duck olayı kullanır.
- Trance Pulse, swing ve rolling ritimler eşit seviyeli kare dizileri değildir; vurgular ve kapılar farklıdır.
- Yumuşak tremolo dışında Rhythm kategorisindeki gerçek gate'ler tam kapanır.
- Stereo sekizlikleri ve Pitch/Saturation üçleme/çeyrek hareketleri adlarına uygun sürelerde çalışır.
- Seçili Stereo rack'lerine Width; Dotted Clouds, Stereo Mist ve uygun Creative rack'lere Send/Feedback/Filter
  eğrileriyle çalışan Delay eklenir. Harici VST3 bağımlılığı yoktur.
- Saturation/Pitch hareket presetlerinin 97 noktalı eğrileri de kompakt segmentlerle değiştirilir.
- Reverb beş alanlı nokta formatı kullanır; clearsTails alanı korunarak Wet/Send ayrı kaydedilir.

## Geliştirme / bakım
Source/DesignedCurves.h: doğrudan düzenlenebilen düğüm tarifleri. v normalize hedef seviyesidir;
Gate/Volume/Filter/Pan/Saturation/Pitch için saklanan Y=1-v. Time motoru Y'yi doğrudan okur.
Reverb her düğümde clearsTails alanı da saklar; formatı Gate ile karıştırmayın.
Source/FactoryPresets.cpp: rack kombinasyonları, derinlik ve tempo.
Source/EnvelopePresets.h: Pitch/Formant/Saturation eğrilerinin eşlenmesi.

## Doğrulama
AudioTests 160 benzersiz rack durumunu dışa aktarır, dosyadan geri yükler ve ses çıktısında
sonlu değerleri kontrol eder. Aktif ana eğrilerde nokta sırası, sınırlar ve 2–40 düğüm denetlenir.
Clean Quarter sabit 0.2 girişle ölçüldü: vuruş boşluğu 0, toparlanma bölgesi 0.2.
Seçili 13 presetin gerçek editör görüntüleri dist/preview-bank-091-*.png olarak oluşturulur.
Son test sonuçları Windows paketindeki test-results.txt içindedir. Sentetik ses testleri her
müzik malzemesinde öznel beğeni veya ses yüksekliği eşitliği garantisi değildir.
