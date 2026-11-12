# Jellybone Yapılacaklar Listesi

Fazıl Irak · 03.11.2026

## Öncelikli hata — Ableton Live 12 A/B geçişinde çökme

Fazıl Irak · 07.11.2026 · Kullanıcı bildirimi; henüz yeniden üretilmedi veya düzeltilmedi.

- [ ] **P1 — A/B geçişinde crash:** Kullanıcı Jellybone'u Ableton Live 12'de kullanırken A ve B butonlarına sırayla birkaç defa bastığında programın çöktüğünü bildirdi. Tam sürüm, rack içeriği, playback durumu ve çökmenin yalnız eklentide mi yoksa host'ta mı olduğu henüz kesinleşmedi; varsayım yapma.
- [ ] Önce kullanıcının senaryosunu yeniden üret; A/B state geri yükleme, rack modüllerinin/editorlerinin ömrü, ses callback'i ile state değiştirme eşzamanlılığı, bekleyen GUI callback'leri ve varsa harici FX eklentilerinin sahipliğini incele. Bunlar araştırma başlıklarıdır, doğrulanmış neden değildir.
- [ ] Nedeni giderdikten sonra tekrarlı A→B→A geçişlerini playback açık/kapalı, farklı rack içerikleri ve varsa harici VST3 ile doğrula. A/B ayarlarının doğru korunduğunu da kontrol et; anlamlı regresyon testi ve gerçek Ableton Live 12 doğrulaması olmadan tamamlandı sayma.

Bu kayıt yalnızca yapılacaklar listesine eklenmiştir; kullanıcı uygulama komutu verene kadar geliştirme/derleme/yayın başlatılmaz.
## Bekleyen doğrulama

- [ ] **FL Studio içinde canlı host testi:** 0.8.7'yi FL Studio'da aç; yalnız sağ alt köşeden orantılı boyutlandırma, yedi Zoom, kontrollerin doğru tıklanması, A/B ve yeniden açılışta boyut/state korunmasını doğrula. Bu bilgisayarın açık/kurulu uygulama envanterinde FL Studio bulunamadı. Test tamamlandı sayılmadı.

## Tamamlanan geliştirmeler — 0.8.7

Hazır Volume presetlerinin taban noktaları, köşe boyutlandırması, ayrı çark simgeli Ayarlar/Zoom paneli, A/B karşılaştırması, Width ve tek VST3 pluginli FX uygulandı.
Ayrıntılar ve kullanım/bakım haritası: docs/YAPILACAKLAR-087-TAMAMLANAN.md.
Önceki 0.8.6 kayıtları: docs/YAPILACAKLAR-086-TAMAMLANAN.md.

## QDelay tabanlı Delay modülü — bekliyor

Fazıl Irak · 06.11.2026 · Kullanıcı isteği: yalnızca listeye eklendi; uygulama daha sonra başlatılacak.

- [ ] tiagolr/qdelay ses motorunu Jellybone rack'ine gömülü Delay efekt türü olarak uyarla. Kaynak: https://github.com/tiagolr/qdelay . İncelenen master derleme sürümü 1.2.3; uygulama başlarken kullanılacak commit'i sabitle.
- [ ] Mevcut Jellybone eğri editörü, tempo senkronu, tetikleme, pattern/sequencer ve ortak kontrol düzenini kullan; kompakt tasarımı ve efekt rengini koru.
- [ ] İlk eğri hedefleri: **Send / Input**, **Mix**, **Feedback**, **Filter**. Hedeflerin ayrı Min–Max aralıklarını, derinliklerini ve yumuşatmalarını tanımla; aynı eğrinin birden fazla hedefi ayrı aralıklarda yönetmesini destekle.
- [ ] **Send / Input:** delay'e yeni ses girişini eğriyle yönet; Send sıfıra indiğinde mevcut tekrarlar ve kuyruk devam etsin. Kuru sinyal yolunu ayrı tut.
- [ ] **Mix:** kuru/efektli karışımını eğriyle yönet. Send ve Mix arasındaki davranış farkını yardım metninde açıkla.
- [ ] **Feedback:** tekrarların geri beslemesini eğriyle yönet; yumuşatma, kontrollü modülasyon sınırları ve aşırı seviye/kararsızlık önlemlerini test et.
- [ ] **Filter:** giriş veya feedback EQ'sunda seçilen frekans hedefini eğriye bağla; frekans ölçeği ve geçiş yumuşatmasını müzikal kullanıma uygun kur.
- [ ] Stereo / ping-pong, tempo senkronu ve nota bölünmeleri ile temel feedback ve filtre ayarlarını koru. QDelay'in diğer özelliklerini (reverse, diffusion, saturation, pitch shifting, ducking, wow/flutter) mevcut motorla birlikte değerlendir; hepsini ilk sürümde eğri hedefi yapmak zorunlu değil.
- [ ] **Delay Time modülasyonu:** gelişmiş/sonraki aşama olarak değerlendir; perde kaymalı tape davranışı ile perdeyi koruyan çapraz geçiş davranışını kararlaştır, tıklamasız geçişleri doğrula.
- [ ] Blok başına parametre okuma yapısını akıcı eğri modülasyonuna uyarla. Sık modülasyonun her seferinde tüm filtreleri/işlemcileri yeniden hazırlamasını önle; ses callback'inde bellek tahsisi/nesne oluşturma/tampon büyütme yollarını gider, tamponları önceden ayır.
- [ ] QDelay C++20 / Jellybone C++17 ve JUCE sürüm farklarını çöz; namespace, kaynak üretimi, kaynak dosyaları ve derleme bağımlılıklarını mevcut gömülü modül yapısına uyarla.
- [ ] Preset/state, A/B, otomasyon ve eski rack oturumlarının uyumluluğunu koru; delay belleği, tail bildirimi, bypass, transport başlangıcı ve render sonunda kuyruğun korunmasını değerlendir.
- [ ] QDelay GPLv3 ve içerdiği diğer yazarlara ait lisans/telif bildirimlerini inceleyip koru; değişiklik ve yazar kayıtlarını, About/lisans sayfalarını ve dağıtılan ikililere eşleşen kaynak arşivini hazırla.
- [ ] Sıfır/düşük/yüksek feedback, Send kapatınca kuyruk, mono/stereo, blok boyutu ve örnekleme hızı, tempo değişimi, hızlı eğri geçişleri, CPU/bellek ve gerçek DAW testlerini yap. Uyumluluk şu an kaynak incelemesine dayanır; derleme ve ses testi henüz yapılmadı.
- [ ] Yardımda kullanım örnekleri ekle: cümle sonunda delay throw, vuruş aralarında duyulan Mix hareketi, feedback yükselişi ve filtreyle koyulaşan tekrarlar.

Not: Harici QDelay'i mevcut FX modülünde yüklemek yalnızca eğri kontrollü dry/wet için alternatif bir yoldur; bu kayıt gömülü ve çok hedefli Delay modülünü amaçlar. Bu görev uygulama/yayın komutu değildir.