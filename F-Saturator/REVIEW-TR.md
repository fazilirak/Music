# Kaynak inceleme notu — 21 Ekim 2026

Kapsam: F Saturator 0.2.0 uygulama kaynakları, DSP testleri, CMake ve iki yerel derleme/paketleme betiği. Bu belge bir güvenlik sertifikası veya kapsamlı lisans denetimi değildir.

## Yapılan düzenlemeler

- 13 uygulama/test/derleme dosyasına 239 satır Türkçe açıklama eklendi.
- C++ kaynaklarının yorumlar dışındaki token dizisi özgün kaynaklarla karşılaştırıldı; aynı kaldığı doğrulandı. Parametre kimlikleri, denklemler, katsayılar, aralıklar ve görünen ürün metinleri değiştirilmedi.
- `GELISTIRME-TR.md` dosya haritasını, sinyal akışını ve güvenli düzenleme adımlarını açıklar.
- `.gitignore`, derleme/dağıtım çıktıları ile yerel anahtar ve ayar dosyalarını Git kapsamı dışında tutar. Bu dışlama, yayımlama öncesi dosya listesini kontrol etmenin yerine geçmez.
- Paket betiği geliştirme/inceleme belgelerini kaynak paketine de alacak şekilde güncellendi.

## İncelemede dikkat çeken bakım noktaları

1. Preset sayısı birkaç yerde 16 olarak sabittir. Yeni preset eklerken isim/değer listeleri, gezinme, sınırlandırma, test ve paket kontrollerini birlikte güncelleyin.
2. JUCE ve Visual Studio için yerel varsayılan yollar bulunur. Yeni bilgisayar için README ve geliştirme rehberindeki açık JUCE yolu kullanılmalıdır.
3. Ekrandaki eğri statik bir yaklaşımdır; DSP'nin manyetik ve dinamik hafızasının tam karşılığı değildir.
4. Auto Gain referans sinüsle kalibre edilir; her kayıt için eşit algılanan ses yüksekliği vaat etmez.
5. Preset dosyası kaydetme/yükleme akışında bütün I/O hataları kullanıcıya ayrı bir hata mesajıyla sunulmuyor. Bu davranış açıklama çalışmasında değiştirilmedi; sonraki işlevsel iyileştirme olarak ele alınabilir.
6. `Resources/fonts/VENDORED.md` başka panel projelerine ait bakım/test notları içeriyor. Üçüncü taraf kaynak bildirimi korunmuştur; F Saturator için uygulanacak test komutu `ctest --test-dir build --output-on-failure` komutudur.

## Doğrulama kaydı

Yorum dışı C++ eşitliği otomatik kontrol edilmiştir. Bu çalışma sırasında yapılan yeniden derleme ve test sonucu ayrıca `dist/test-results.txt` ve teslim klasöründeki doğrulama kaydında tutulur. Test başarısı, tüm DAW ve bilgisayarlarda doğrulama yapıldığı anlamına gelmez.
