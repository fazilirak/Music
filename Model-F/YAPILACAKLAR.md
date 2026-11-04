# Model F — Yapılacaklar

Fazıl Irak · Oluşturulma: 25.10.2026

## Çalışma düzeni

- Kullanıcı “listeye ekle” dediğinde ilgili hata veya istek bu dosyada biriktirilir.
- Her madde için ayrı derleme ve yayın yapılmaz.
- “Yapılacak listesini tamamla” komutu geldiğinde biriken maddeler topluca uygulanır ve uygun testlerle kontrol edilir.
- Tamamlanan maddeler listeden çıkarılır; bitmeyen maddeler gerekçeleriyle korunur. Tümü tamamlandığında liste boşaltılır.
- GitHub ve web yayını, kullanıcının yayın talebine göre yapılır.

## Öncelikler — Sylenth1 karşılaştırması sonrası, 04.11.2026

Önce mevcut davranışları sağlamlaştır, CPU yükünü azalt ve preset kalitesini tutarlı hale getir. Yeni özellikleri bu çalışmaların ardından değerlendir. Aşağıdakiler plan maddeleridir; uygulanmış veya tamamlanmış sayılmaz.

- [ ] **P1 — Güvenilirlik ve davranış tutarlılığı.** Preset değiştirme, ilk modülasyon düzenlemesi, hedefler arasında geçiş, Status aç/kapat, reset ve eski/yeni sistem geçişlerini birlikte doğrula. Bir hedefteki düzenlemenin diğer kapalı hedefleri açmadığını ve görünümün gerçek parametrelerle eşleştiğini kontrol et. Bulunan hatalar için davranışı tekrar üreten regresyon testleri ekle.
- [ ] **P1 — Gerçek DAW ve oturum doğrulaması.** Ableton içinde MIDI, otomasyon, preset yükleme, proje kaydetme/yeniden açma, birden çok eklenti örneği ve uzun kullanım senaryolarını dene. Otomatik testlerin geçmesini kullanıcı dinlemesi veya bütün kullanım yollarının doğrulanması olarak sunma.
- [ ] **P1 — CPU profilini çıkar ve ölçerek iyileştir.** Özellikle SuperSaw, yüksek polifoni, 96 kHz ve tüm modülatörlerin açık olduğu durumları incele. Kullanılmayan kaynakların hesap yükünü, osilatör/filtre maliyetini ve çoklu örnek davranışını ölç. Kazancı aynı koşullarda önce/sonra ölç; ses, zamanlama ve otomasyon doğruluğunu koru. Offline tek çekirdek yüzdesini DAW'ın CPU göstergesiyle karıştırma.
- [ ] **P2 — Preset bankasının kalite ve tutarlılık incelemesi.** 160 fabrika sesini kategori bazında dinle; çıkış seviyesi, kullanılabilir headroom, rezonans, nota aralığı, velocity, nota bırakma ve modülasyon durumlarını kontrol et. Eski açıklamaları ve yeni panelle uyuşmayan davranışları gözden geçir. Yalnız preset sayısını artırmak yerine seslerin kullanım değerini ve kalite tutarlılığını yükselt.
- [ ] **P2 — Sylenth1 ile kontrollü ses karşılaştırması.** Aynı MIDI, aynı örnekleme hızı, benzer patch ve eşit algılanan seviyede bas, lead, pluck, pad ve SuperSaw örnekleri hazırla. Önce efektler kapalı kuru sesi, sonra aynı harici efekt zinciriyle sonucu karşılaştır. Mümkünse kör A/B dinleme yap; devre modellemesi veya özellik sayısından ses üstünlüğü sonucu çıkarma. Mevcut seviye eşlenmiş dinleme maddesiyle birlikte tamamla.
- [ ] **P2 — Sylenth1 ile karşılaştırmalı CPU ölçümü.** Aynı bilgisayar, DAW, buffer, örnekleme hızı, MIDI, benzer polifoni/unison ve efekt koşullarında ölçüm yap. Mimari farkları ve eşleştirilemeyen ayarları raporla; ölçüm yapılmadan daha hafif/ağır olduğuna ilişkin sayısal iddia üretme.
- [ ] **P3 — Sonraki özelliklerin kapsamını değerlendir.** Efekt bölümü, daha genel modülasyon yönlendirmesi, katmanlı osilatör ve çift filtre ihtiyacını; kullanım faydası, CPU maliyeti ve arayüz sadeliği üzerinden değerlendir. Model F'nin hedef başına Multi EG/LFO/Step avantajını koru. Bu madde, karşılaştırmada geçen bütün Sylenth1 özelliklerini ekleme kararı değildir; uygulama kapsamı ayrıca belirlenecek.

GitHub ve web güncellemeleri için kullanıcının ayrıca onayı beklenir.

## Bekleyen doğrulama

- [ ] **Ses gövdesinin kullanıcı/DAW dinleme onayı.** Full çıkış ve waveform düzeltmesi uygulandı; 72 test ve 24 seviye ölçümü geçti. Sylenth1, Serum veya UAD Minimoog ile aynı MIDI/benzer patch ve seviye eşlenmiş dinleme henüz yapılmadı. Kaynak/model ölçümleri öznel kalite onayı yerine geçmez.
- [ ] Model F 0.2.9'un gerçek DAW içinde Ctrl+sürükle ile sekiz waveform ve Full/Legacy geçişini kontrol et. Canlı uygulama açma izni zaman aşımına uğradığı için bu kontrol tamamlandı sayılmadı.

## Tamamlananlar

03.11.2026: Waveform/fırça düzeltmesi, Full/Legacy çıkış geliştirmesi ve yerel Windows paketi hazır.
Ayrıntılı sonuçlar: docs/YAPILACAKLAR-029-TAMAMLANAN.md.
Önceki liste: docs/YAPILACAKLAR-029-ONCEKI.md.
GitHub/web yayını kullanıcı talebine göre ayrıca yapılır.
