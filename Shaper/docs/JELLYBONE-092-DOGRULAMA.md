# Jellybone 0.9.2 — doğrulama ve kalan kullanıcı kabulü
Fazıl Irak · 19.11.2026 · Yerel Windows x64 VST3 / Standalone

## Düzeltilen uzun Delay kuyruğu bildirimi
Rack ve MultibandProcessor önceki sürümde sabit 30 saniye kuyruk bildiriyordu. Delay'in 4 saniye / feedback 0.9 ayarı yaklaşık 356 saniyelik korumacı bir kuyruk süresi verir; eski rack bunu host'a aktarmıyordu. Host bu bilgiyle render süresini sınırlarsa kuyruk erken kesilebilirdi. Eski kaynakta eklenen test 'Rack tail must cover configured Delay tail' hatasıyla sorunu doğruladı.
0.9.2'de paralel bantların en uzun kuyruğu rack'e aktarılır; seri ve etkin/present modüllerin kuyrukları toplanır. Rack'in önceki 30 saniye alt sınırı korunur. Bypass edilmiş Delay süreyi uzatmaz; pasif bantların uzun süreleri de mod otomasyonuna karşı korumacı üst sınıra dahildir. Parametre ID, state v8 ve eğri/ses işleme algoritmaları değiştirilmedi. Bildirilen süre render'ın mutlaka o kadar sessiz uzatılması gerektiği anlamına gelmez; bazı DAW'lar kullanıcı export aralığını esas alır.

## Otomatik doğrulama
- Release Windows VST3, Standalone ve altı test hedefi derlendi. İki Windows ikilisi de 0.9.2 sürüm kaynağını taşır.
- CTest 6/6 geçti, toplam 50.72 saniye. audio_integration, irak_dsp, pitch_dsp, curve_effects, utility_effects ve delay_effect; kesin kayıt acceptance-2026-11-19/CTest-0.9.2.log.
- Son VST3 pluginval strictness 10 SUCCESS: acceptance-2026-11-19/pluginval-0.9.2.txt.
- Low/Mid/High kuyruk maksimumu, seri FX + Delay toplamı, bypass ve oturumdan geri çağrılan uzun kuyruk için yeni regresyonlar geçti.
- Önceki A/B ömür regresyonları tekrar geçti: 44 editör/banka geçişi, 30 eşzamanlı audio/state geçişi, kurulu F-Saturator ile 12 A/B state/audio geçişi. Bu, kullanıcının bilinmeyen özgün projesi ve tüm üçüncü taraf FX'ler için garanti değildir.
- Yedi Zoom, köşe handle'ı/kenar resize kapalı durumu, %75 açılış ve son ölçünün kalıcılığı, Width/FX ve mevcut factory preset regresyonları geçti. Gerçek Delay ve Zoom görüntüleri görsel olarak incelendi; Paint/Seq/Curve satırı açıktır.

## Gerçek derlenmiş VST3 ile uzun çevrimdışı host denemesi
Tests/DelayAcceptance.h, test edilen Jellybone.vst3 ikilisini JUCE VST3 hostunda açar. Kaydedilmiş A/B bankalı Delay+Width state'i yüklenir. Dokuz koşul: 44.1/48/96 kHz × 64/257/1024 örnek. Sekiz koşul 12 sanal ses saniyesi, 48 kHz/257 koşulu 300 sanal ses saniyesi işler. Gerçek zamanlı beş dakika bekleme veya Ableton oturumu değildir.
Özgün stereo akor/vuruş girdisi, MIDI note-on, 90→135→180 BPM host PPQ, transport stop, host gesture üzerinden Feedback/Filter otomasyonu, ortada state kaydet/geri yükleme ve sonlu stereo/headroom denetimi çalışır. Çökme, sessiz tüm çıktı veya sonlu olmayan örnek oluşmadı.

VST3-DELAY-MATRIX.csv ölçülen callback sürelerini olduğu gibi içerir. 48 kHz/257 örnek/300 saniye koşulunda ortanca 0.0609 ms, p95 0.0905 ms; blok bütçesi 5.3542 ms. Bu koşulda ve diğer sekiz koşulda ölçülen aşım 0. Yalnız test rack'inin yerel çevrimdışı duvar saati ölçümüdür; Ableton CPU yüzdesi, donanım underrun sayısı veya her rack/sistemde gerçek zaman garantisi değildir. Bu testte tek instance kullanıldı; çoklu DAW instance testi yapıldığı iddia edilmez. Raporlanan gecikme bu fixture'da 0 örnek, kuyruk 118 saniye.
Delay-original-phrase.wav özgün 12 saniyelik işlenmiş stereo örnektir; ticari preset/sample kullanılmadı. Burada kulakla müzikal beğeni onayı verilmedi.

## Canlı uygulama kontrolü ve erişim sınırları
Ableton Live 12 Standard açıldı, kurulu Jellybone 0.9.1 boş Audio 3 kanalına yüklenip arayüzü görüldü. Bu kurulu dosya 0.9.2 ile değiştirilmedi. Menü/preset diyaloğunda bilgisayar aracı 'coordinate input geometry is unavailable' ve 'element ... is not available in cached app state' hataları verdi. Açılan dosya diyaloğu Escape ile kapatıldı. Uzun sesli oturum veya son 0.9.2'nin canlı kabulü tamamlanmış sayılmaz; kullanıcı projesi değiştirilmedi, boş test seti kaydedilmedi.
Kurulu uygulama listesinde FL Studio yok; standart Image-Line kurulum dizinleri bulunmadı. FL Studio yüklenmedi veya satın alınmadı. Özgün çökme projesi/FX listesi kullanıcıdan soruldu; gelmeden o projede doğrulama yapılamaz.

## Kalanlar
1. Erişilebilir FL Studio'da köşeden orantılı resize, yedi Zoom, hit-test, A/B ve kapat/aç ölçü/state denemesi.
2. Son 0.9.2 ile gerçek Ableton/FL Studio ses/MIDI klibi, farklı aygıt buffer/tempo, uzun oturum ve export kuyruğu kontrolü. Yeni çevrimdışı test bu maddenin teknik kapsamını genişletir, canlı kabulün yerini almaz.
3. Kullanıcının özgün Ableton projesi ve kullandığı üçüncü taraf FX'lerle A/B çökme senaryosunun tekrar denemesi.

Kaynaklar backups/2026-11-19-acceptance altında yedeklendi. Yerel Windows ikili ve eşleşen tam kaynak paketi hazırlandı. GitHub/web/LinkedIn/abonelere yayın yapılmadı. macOS/Linux bu görevde derlenmedi.
