# Model F — Yapılacaklar

Fazıl Irak · Oluşturulma: 25.10.2026

## Çalışma düzeni

- Kullanıcı “listeye ekle” dediğinde ilgili hata veya istek bu dosyada biriktirilir.
- Her madde için ayrı derleme ve yayın yapılmaz.
- “Yapılacak listesini tamamla” komutu geldiğinde biriken maddeler topluca uygulanır ve uygun testlerle kontrol edilir.
- Tamamlanan maddeler listeden çıkarılır; bitmeyen maddeler gerekçeleriyle korunur. Tümü tamamlandığında liste boşaltılır.
- GitHub ve web yayını, kullanıcının yayın talebine göre yapılır.

## Bekleyen maddeler

- [ ] **Global Tuning — genel akort ayarı** (26.10.2026)
  - Ana ekranda, Multi-Stage Envelope butonunun solunda, aynı satırın en sol başında küçük bir ayar alanı bulunacak.
  - Referans görseldeki GForce Axxess Global Tuning alanı gibi, değerin üzerine tıklayıp fareyi yukarı/aşağı sürükleyerek akort ayarlanacak; güncel değer alanda gösterilecek.
  - Ayar presetlerden bağımsız olacak; preset seçimi veya yükleme global tuning değerini değiştirmeyecek.
  - Model F'nin mevcut arayüzüne uyumlu, kompakt bir yerleşim kullanılacak.
  - Referans: Kullanıcının 26.10.2026 tarihli GForce Axxess ekran görüntüsünün en alt solundaki Global Tuning alanı.

- [ ] **Character — ses motoru karakter seçicisi** (26.10.2026)
  - Küçük, yatay bir seçim alanıyla Original / Ladder / Edge / Bite / State Variable karakterleri arasında geçiş yapılacak; mevcut kompakt arayüz korunacak.
  - Original: mevcut Model F sesini ve eski preset/oturum davranışını koruyacak.
  - Ladder: 24 dB/oktav ladder yaklaşımı, filtre giriş sürüşü ve rezonans etkileşimiyle dolgun bass, yuvarlak filtre tepkisi ve yoğun lead hedeflenecek.
  - Edge: referans ARP Odyssey olacak; Odyssey'nin iki osilatör, sync, ring modulation, PWM ve sample & hold yaklaşımı dikkate alınacak. Keskin sync lead'leri, belirgin ataklar ve metalik hareketler hedeflenecek. Odyssey revizyonlarının farklı filtreleri bulunduğu gözetilerek uygulanacak filtre sürümü geliştirme aşamasında açıkça belirlenecek.
  - Bite: MS-20 yaklaşımından esinlenen, rezonanslı high-pass ve low-pass filtrelerin etkileşimi ve nonlinear sürüşüyle agresif karakter hedeflenecek; yalnız mevcut rezonans değerini artırmakla yetinilmeyecek.
  - State Variable: SEM yaklaşımından esinlenen 12 dB/oktav çok modlu state-variable filtre; low-pass, high-pass, band-pass ve notch tepkileriyle açık tonlar ve hareketli pad/akor karakteri hedeflenecek.
  - Bunlar özgün algoritmalarla geliştirilen ses hedefleri olacak; birebir donanım emülasyonu veya ilgili üreticilerle resmî ilişki iddiası kullanılmayacak. Arayüzde marka adları yerine yukarıdaki teknik/özgün isimler kullanılacak.
  - Seçim yalnız bir EQ/preset değişimi olmayacak; ilgili filtre mimarisi, rezonans, doygunluk ve gereken osilatör/zarf davranışları farklılaşacak. Mevcut ana kontroller yerlerinde kalacak; ek filtre kontrolü gerekirse mümkün olduğunca küçük ve bağlama uygun tasarlanacak.
  - Geçişler klik oluşturmadan kısa ve yumuşak yapılacak. CPU yükü için sürekli bütün motorlar yerine seçili motor çalıştırılacak; geçişte gerekirse kısa süreli çapraz geçiş kullanılacak.
  - Karakter seçimi preset ve DAW oturumuna kaydedilecek; eski kayıtlarda varsayılan Original olacak. Global Tuning'in presetlerden bağımsız olması ayrı madde olarak korunacak.
  - Karakter farkları seviye eşlenmiş dinleme ve uygun DSP testleriyle değerlendirilecek; sayısal kararlılık, CPU yükü, otomasyon ve preset uyumluluğu kontrol edilecek.

- [ ] **CPU kullanımı — ölçüm ve optimizasyon incelemesi** (26.10.2026)
  - Model F'nin CPU kullanımını azaltmanın mümkün olup olmadığı incelenecek; optimizasyon öncesinde ses motoru ve arayüzdeki darboğazlar ölçülecek.
  - Mono, Poly ve SuperSaw; farklı aktif nota sayıları, örnekleme hızları, blok boyutları ve modülasyon ayarlarıyla karşılaştırılacak. Yeni karakter motorları da eklendiğinde bu incelemeye dahil edilecek.
  - Kullanılmayan/sessiz seslerin işlemleri, pahalı örnek başına hesaplamalar, filtre katsayısı güncellemeleri ve arayüz yenilemeleri gözden geçirilecek; ölçülen faydaya göre iyileştirilecek.
  - Ses kalitesi, akort, envelope zamanlaması, otomasyon, preset uyumluluğu ve gerçek zamanlı kararlılık korunacak. Ses karakterini değiştiren kalite azaltımları sessizce uygulanmayacak.
  - Öncesi/sonrası aynı koşullarda ölçülerek gerçek kazanım ve sınırlamalar raporlanacak; ölçüm yapılmadan belirli bir CPU düşüşü taahhüt edilmeyecek.

