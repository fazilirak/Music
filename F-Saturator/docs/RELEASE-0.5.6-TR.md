# F-Saturator 0.5.6 — Görünüm ve CPU güncellemesi

Fazıl Irak · 19 Kasım 2026

## Değişiklikler

- Alt preset satırındaki **UI** düğmesinden %50, %75, %100, %125, %150, %175 ve %200 ölçek seçilir. Varsayılan kayıt yoksa %100, 912 × 511 açılır.
- Sağ alt köşe ile orantılı boyutlandırma yapılabilir. Son boyut ve seçilen ölçek kullanıcının bilgisayarında saklanır; editör ve DAW yeniden açıldığında okunur. Ses presetine veya DAW ses parametrelerine yazılmaz.
- Açılışta ve görünür pencere başka ekran/DPI alanına taşındığında kullanılabilir ekran sınırları uygulanır. Büyük ölçek seçimi ekrana sığacak boyuta sınırlandırılır; DAW çerçevesi için pay bırakılır.
- ADAA katsayıları ve önceki örneğin antitürev sonucu yalnız geçerli kaldıklarında tekrar kullanılır. Katsayı değişince önbellek geçersizleşir. Drift sıfırdayken duyulmayan sinüs hesapları atlanır, faz ilerlemeye devam eder.
- Timer bütün pencereyi her karede boyamaz. Metre alanlarını günceller; transfer eğrisi yalnız ilgili kontrol değişince çizilir. Ses işleminde dosya erişimi veya yeni kilit eklenmedi.
- VST3 hostunun aç/kapa parametresine kesirli değer göndermesi halinde parametre okuması iki adımlı sözleşmeye uygun 0/1 verir; durum geri yükleme testinde bulunan tutarsızlık düzeltildi. Kimlikler ve preset biçimi korunur.
- 8× FIR oversampling, ADAA, filtreler, model geçişleri, gecikme ve ses kalitesi korunur.

## Doğrulama

- Release Windows x64 VST3 ve Standalone derlendi; sürüm kaynak ve iki ikilide 0.5.6.
- CTest: irak_dsp, irak_view, irak_reference geçti (3/3).
- pluginval strictness 10, seed 0x19112026: SUCCESS. Editör açma/kapama, işlem sırasında editör, state restore, otomasyon, farklı sample rate/bloklar ve fuzz kontrolleri dahil.
- 0.5.5 referans motoruyla 44.1/48/96/192 kHz, beş model, model geçişleri, parametre rampaları, harmonikler, Drift, desk katı ve bypass senaryolarında **2.621.440 örnek karşılaştırıldı: en büyük fark 0**.
- Yedi ölçek, yeniden oluşturulmuş processor/editörle boyut hatırlama, ses presetinden ve session state yüklemesinden bağımsızlık, hatalı/aşırı büyük kayıt, orantılı yerleşim ve ikili parametreler otomatik sınandı. Görseller gerçek editörden üretildi.
- İşlemci: Intel Core i9-13900H. CPU testi eski/yeni motoru aynı süreçte, aynı benchmark iş parçacığını aynı çekirdeğe sabitleyerek dönüşümlü çalıştırır; her durumda üç tekrarın medyanı alınır. Stereo, 18 dB drive, 0.12 bias, 0.2 character, 0.3 harmonics; 44.1/48/96 kHz ve 64/256 örnek bloklar. Sonuçlar **CPU-0.5.6.csv** dosyasındadır. Süreler ms/s ses cinsindedir; DAW CPU yüzdesi değildir.
- Arayüzde sürekli tam repaint ve clipping dışında kalan 257 noktalı eğri üretimi kod incelemesiyle ayrıştırıldı ve kaldırıldı. Ayrı bir GUI/GPU zaman serisi ölçümü yapılmadı.

## Sınırlar / dış doğrulama

- Geri bildirim sahibinin CPU, sample rate, buffer ve preset bilgileri istendi; henüz gelmedi. Kullanıcı mevcut bilgilerle devam edilmesini istedi.
- Bildirilen Ableton Live 12.4.3 kurulu değil; bilgisayardaki Live sürümü 12.4.16. Bu sürümde tam DAW oturumu kaydet/kapat/yeniden aç ve gerçek sorunlu preset karşılaştırması yapılmış sayılmıyor. Canlı Standalone kontrolü kullanıcı tarafından Esc ile durduruldu. Otomatik editör/state testleri ve pluginval tamamlandı.
- Fiziksel çoklu ekran/DPI taşıma ve ses kartı sürücüsüyle canlı kullanım ayrıca doğrulanmalı. Otomatik ekran sınırı koruması uygulandı.
- Bu yerel Windows sürümüdür. Yeni macOS/Linux ikilisi, GitHub yayını, web güncellemesi ve abonelere e-posta bu iş kapsamında yapılmadı.

Yerel DSP testlerindeki süre azalması: en düşük %6.78, en yüksek %33.20, medyan %21.87. Sistem yükü ve ayarlar sonucu etkiler; bütün makineler veya presetler için garanti değildir.

Dependency audit: the cached JUCE build tree matches every bundled pinned source file after CRLF/LF normalization (1,359 line-ending-only differences; zero content differences). Bundled upstream SHA-256 verification is also required by the packager.
