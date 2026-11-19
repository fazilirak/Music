# F-Kick 0.8.7 — sayı gösterimi ve giriş hassasiyeti

Fazıl Irak · 03.11.2026

Görünür sayılar en fazla iki ondalık haneye sahiptir. DAW'ın genel parametre metinleri de bu sınıra uyar; gereksiz sondaki sıfırlar kaldırılır. Sample Start/End grafik süreleri artık üç yerine iki ondalık gösterilir.

- Nota numarası, semiton seçimleri, faz derecesi, tempo, filtrelerin Hz değerleri ve uzun zaman değerleri tam sayı adımlıdır.
- Attack süresi, gövde temel frekansı, kazanç/dB, Q, fine bass tune ve saniye değerleri 0,01 hassasiyetlidir.
- Yüzde miktarları bir yüzde adımıyla; sample Start/End ise yüzde 0,01 adımıyla düzenlenir. İç saklama birimleri değişmez.
- Attack/Pump/Sustain knobları tam sayı yüzde adımlıdır ve çift tıklamayla sıfırlanır.
- Eski preset, state ve otomasyon sayıları topluca yuvarlanmaz. Bu hassasiyet kuralı yeni kullanıcı girişleri içindir; mevcut ses ve eğri çözünürlüğü korunur. Parametre kimliği, sıra ve aralıkları değiştirilmedi.

Bakım: Source/NumericPolicy.h ortak politika; PluginEditor.cpp sayı metinleri ve kullanıcı girişleri; PluginProcessor.cpp DAW metin dönüşümleri; TransientKnob.h yüzde adımları.

Doğrulama: tüm float host metinleri için 0/0,1234567/0,7891234/1 normalize değerlerinde en fazla iki ondalık; gerçek editöre tempo, root, Start ve gain yazarak geri okuma; eski state ve tüm ses/transient regresyonları. CTest-087.log son çalıştırmanın kaydıdır. Windows VST3 ve Standalone yerel paketi; diğer işletim sistemleri için bu sürüm derlenmedi veya yayımlanmadı.
