# Jellybone 0.8.7 — Geliştirme ve kullanım notları

Fazıl Irak · 3 Kasım 2026

## Arayüz ve karşılaştırma

- Pencere yalnız sağ alt tutamaçtan sabit en-boy oranıyla boyutlandırılır. Host kenar boyutlandırması kapalıdır.
- Üst sağ sıra A, B, çark simgeli Ayarlar ve (i) About şeklindedir.
- Ayrı Ayarlar panelinde %50, %75, %100, %125, %150, %175, %200 Zoom seçenekleri vardır. %100 temel tuval 1100 x 960'tır. Varsayılan/önceden kaydedilmiş tercih korunur.
- A/B ilk geçişte mevcut ayarları diğer bankaya kopyalar. Sonraki düzenlemeler her bankada ayrı kalır. Efekt sırası, parametreler, eğriler, preset adı ve harici plugin state'i bankaya dahildir. UI boyutu A/B ile değiştirilmez.
- DAW oturumu iki bankayı ve seçili bankayı saklar. Fabrika preset yüklemek pasif karşılaştırma bankasını silmez.

## Preset tabanı

Sekiz hazır Volume eğrisindeki alt taban noktaları grafik Y=0 seviyesine indirilmiştir. Gate eğri biçimi Y'yi ters saklar: görünen taban Y=0, XML'de Y=1'dir. Metin veri tablosu ile Source/Embedded/VolumePresets içindeki fabrika dosyaları birlikte güncellenmiştir. Kullanıcının kendi dosyaları değiştirilmez. Pitch/Pan gibi bipolar ses tasarımlarının merkezleri keyfi olarak aşağı taşınmaz.

## Width

Pitch/Gate ile aynı düzenleyici, tempo, MIDI/audio tetikleme ve sequencer yapısı kullanılır. Min width ve Max width eğrinin iki ucunu belirler: 0% mono, 100% orijinal stereo, 200% iki kat Side. Mid/Side işlemi mono toplamını korur; mono girişten yapay stereo üretmez. Genişletme bazı stereo kayıtların tepe seviyesini yükseltebilir. Ortak çıkış panelindeki Mix/Out/Stereo kontrolleri de çalışır. Low/Mid/High bantlar bağımsızdır.

## FX — tek VST3

FX kartında Load VST3 ile mevcut .vst3 dosyası veya bundle seçilir. İlk sürümde otomatik tüm-disk plugin taraması yoktur. Edit plugin seçilen efektin kendi arayüzünü açar. Modülde yalnız bir VST3 instance bulunur; henüz birden fazla FX kartı eklenmez. Width dahil diğer kartlarla birlikte sıralama yapılabilir.

Eğri harici pluginin kendi Mix parametresine yazılmaz; Jellybone kuru/işlenmiş sesi karıştırır. Min Mix ve Max Mix sınırları 0..100%'dür. Reverb/delay gibi efektleri kendi içinde tamamen wet ayarlamak, efekt miktarını Jellybone eğrisiyle kontrol etmeyi kolaylaştırır. Plugin sessiz eğri kısmında da çalışmaya devam eder; kuyruklar/işlemci durumu kesilmez. Çıkışta duyulan kuyruk miktarı eğriyle azalabilir.

Stereo VST3 ses efektleri desteklenir. Enstrüman veya Jellybone'un kendisi yüklenmez. Plugin ikilisi/lisansı pakete eklenmez; diğer bilgisayarda da aynı pluginin kurulu olması gerekir. Eksik plugin geri çağrıldığında kuru sinyal geçer ve durum alanında eksik plugin adı gösterilir. CPU yükü seçilen efekte bağlıdır; tüm üçüncü taraf pluginlerin uyumluluğu garanti edilmiş değildir.

Gecikme telafisi için iki saniyelik önceden ayrılmış tampon vardır. Plugin yükleme/silme dosya ve GUI yollarında yapılır; callback bu işlemleri bekleyen kilide girmez. Dinamik gecikme değişimleri ve farklı host/üçüncü taraf plugin kombinasyonları ayrıca denenmelidir. VST3 yükleme, ses işleme, dry/wet uçları ve state geri çağırma gerçek F-TransForm ile test edilir.

## Bakım haritası

- Source/Editor.*: köşe tutamacı, Zoom, Ayarlar ve A/B düğmeleri.
- Source/Processor.*: A/B snapshot'ları, sürüm 7 state göçü ve eski sıra kimliklerini koruyan yeni Width/FX konumları.
- Source/UtilityEffect.*: Width Mid/Side, VST3 yükleme/state/UI, gecikme ve kuru/işlenmiş karışım.
- Source/MultibandProcessor.*: Width dahil ortak bant/çıkış kontrolleri.
- Tests/UtilityTests.cpp: Width uçları/mono toplamı, A/B kalıcılığı, yedi Zoom, ayar paneli, gerçek hosted VST3 testleri.
- Tests/AudioTests.cpp: eski efektler, fabrika presetleri, yeni Width çıkış kontrol sözleşmesi ve geriye uyumluluk.

FL Studio canlı host testi, bu ortamda uygulama bulunamadığı için henüz tamamlanmamıştır. JUCE test editöründe boyut/hit-test/state doğrulanır; bu kayıt FL Studio sonucu yerine geçmez.
