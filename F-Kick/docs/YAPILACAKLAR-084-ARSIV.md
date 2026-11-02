# F-Kick 0.8.4 — Tamamlanan işler

Fazıl Irak · 03.11.2026

Kaynak uygulaması, otomatik doğrulama, 12 preset/WAV ve kompakt GUI hazırlandı. Test kanıtları validation/TEST_SONUCLARI_TR.md, kullanım docs/RUMBLE-084-TR.md. Aşağıdaki özgün listenin kulakla dinleme/gerçek DAW doğrulama kısmı henüz tamamlanmadı; aktif listede korunur. Programlama maddeleri tamamlandı.

# F-Kick — Yapılacaklar

Fazıl Irak · Oluşturulma: 30.10.2026

## Çalışma düzeni

- “Listeye ekle” talepleri burada biriktirilir; tek tek derleme veya yayın yapılmaz.
- “Yapılacaklar listesini tamamla” talebi geldiğinde maddeler topluca uygulanır, uygun testlerle doğrulanır ve yerel paket hazırlanır.
- Tamamlanan maddeler doğrulama notuyla arşivlenir; bitmeyen maddeler gerekçeleriyle listede kalır. Tümü tamamlandığında bekleyen liste boşaltılır.
- GitHub ve web yayını yalnızca kullanıcının yayın talebiyle yapılır.
- Bu dosya planlama içindir; aşağıdaki özellikler henüz uygulanmış değildir.

## Bekleyen maddeler

### 1. Öncelik — Efekt dönüşlerini bağımsız yönetme

- [ ] Rumble, Echo ve Room dönüşlerinin seviye ve dinamik yönetimini daha bağımsız hale getir. Mevcut parametre kimliklerini ve eski preset/DAW oturumlarının sesini koru.
- [ ] Mevcut Duck ve SC'nin birleşik efekt dönüşüne uygulandığı davranışı eski kayıtlar için koru. Yeni kullanımda her dönüşün ducking/ritmik şekillendirmeye katılımını bağımsız yönetebilecek bir tasarım hazırla; ana kick ve sample doğrudan etkilenmesin.
- [ ] Rumble'ın ani tepe ve seviye birikimini kontrol etmek için ayrı dinamik kontrol/limitleme ihtiyacını değerlendir. Son çıkıştaki örnek sınırlandırmasını bir dinamik limiter yerine sunma. Ek işleme nötr veya bypass durumda önceki sesi korusun.

### 2. Classic — Mevcut karakteri koruma

- [ ] Mevcut dört comb tabanlı rumble motorunu Classic yaklaşımı olarak koru. HP → Drive → LP, Amount/Gain, Decay, Echo kaynağı ve mevcut Duck/SC işleyişini belgeleyerek referans sesler oluştur.
- [ ] Yeni yaklaşımlar eklenirken Classic sesinin değişmediğini karşılaştırmalı render ile doğrula. Üç yaklaşımı yalnızca farklı reverb presetleri olarak sunma.

### 3. Groove — Ritmik rumble yaklaşımı

- [ ] Rumble/Echo dönüşlerinde tempo senkronlu ritmik hareketi geliştiren ayrı Groove yaklaşımı tasarla. Echo zamanlaması ile daha ayrıntılı dönüş seviyesi eğrilerini birlikte kullan; kick vuruşuna yer açan, vuruş aralarını dolduran çeşitli hareketler üret.
- [ ] Mevcut dört SC eğrisi ve Shift/Mix davranışını koruyarak daha ayrıntılı rumble şekillendirmesinin kompakt arayüzde nasıl sunulacağını belirle. Eski LFO/AD editörünü geri getirmeyi zorunlu çözüm olarak kabul etme.
- [ ] Videodaki audioya kaydetme, parça seçme ve DAW'da yeniden düzenleme iş akışını kullanım örneğiyle açıkla. Dahili audio dilimleme/sequencer bu aşamada taahhüt edilmez; gerekirse ayrıca kapsamlandırılır.

### 4. Hybrid — Ayrı bass katmanı

- [ ] Kick ve rumble'a ek olarak bağımsız, akort edilebilir ve ritmik çalabilen bir rolling/driving bass katmanı tasarla. Kick kuyruğunu uzatmayı ayrı bass motoru gibi sunma.
- [ ] Kick, rumble ve bass arasında frekans/görev paylaşımını ve ducking ilişkisini düzenle. Gerektiğinde kick'i kısa tutup devam eden hareketi ayrı katmanlara bırakabil.
- [ ] Bass katmanının nota/tetikleme ve tempo davranışını, mono/sub uyumluluğunu ve parametre kapsamını uygulama öncesinde netleştir; kompakt F-Kick iş akışını koru.

### 5. Arayüz, presetler ve doğrulama

- [ ] Classic / Groove / Hybrid seçimini mevcut 912 × 432 kompakt paneli koruyarak tasarla; gereksiz büyük knob grupları ekleme. Arayüz İngilizce, bakım açıklamaları Türkçe olsun.
- [ ] Üç yaklaşımı gerçek kullanımda ayıran özgün presetler ve örnek sesler hazırla: kick başlangıcının açıklığı, vuruşlar arasındaki groove ve hybrid bass katmanı ayrı ayrı duyulabilsin. Üçüncü taraf ses/preset/kod kopyalama.
- [ ] Dönüşleri ayrı ve birlikte dinleyerek tepe birikimi, DC/sub yığılması, stereo/mono davranışı, tempo değişimi, tetikleme ve preset/oturum geri çağırmayı kontrol et. Sesi dinlemeden kalite iddiası kurma.
- [ ] Gerçek zamanlı ses yoluna dosya işlemi, bellek ayırma, bekleyen kilit veya GUI çağrısı ekleme. CPU/gecikme etkisini ölç, CTest ve gerçek GUI kontrollerini yap; sonuçları belgeleyip yerel Windows paketini hazırla.

## Mevcut motorun referansı

30.10.2026 tarihinde okunan kaynaklara göre:

- Kaynak: sentez gövdesi + sample, ana filtre/drive sonrasında efekt motoruna girer.
- Rumble: her kanalda dört kısa geri beslemeli comb → HP → seçili distortion → LP → dönüş seviyesi.
- Echo: kick veya işlenmiş rumble'dan beslenir; ayrı paralel dönüş oluşturur.
- Room: ayrı paralel reverb dönüşüdür.
- Birleşik efekt dönüşü FX HP/LP ve SC ile şekillenir, Duck uygulanarak kuru sesle birleşir. Sonraki genel HP ve Bass EQ tüm birleşik sinyali etkiler.
- Bağımsız nota/pattern çalan bir hybrid bass motoru ve dahili audio dilimleme düzenleyicisi henüz yoktur.
- Kaynaklar: Source/TechnoFX.cpp, Source/TechnoFX.h, Source/FXShaper.h, Source/Engine.cpp.