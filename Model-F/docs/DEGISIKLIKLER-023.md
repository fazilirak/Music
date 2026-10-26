# Model F 0.2.3 — Toplu arayüz düzenlemesi

Fazıl Irak · 25.10.2026

- Multi-Stage Envelope açıkken Amount düğmesi, etiketi ve Target alanı turuncu gösterilir.
- Sound Library açıkken Multi-Stage düğmesi görsel ve etkileşim olarak pasiftir.
- Multi-Stage Envelope açıkken üst preset alanı, oklar, Save, Load, Export ve Output pasiftir. Kapatılınca kontroller yeniden etkinleşir.
- User sekmesindeki preset satırlarının sağında silme düğmesi vardır. DELETE onayı dosyayı Presets/.deleted altındaki ayrı klasöre taşır. Cancel veya ESC vazgeçer. Fabrika sesleri silinemez.
- Silme işlemi dosyanın kütüphane içinde olduğunu, normal dosya olduğunu ve onay öncesinden beri değişmediğini kontrol eder. Çalınan sesi/preset parametrelerini değiştirmez.
- Geri alma: .deleted alt klasöründeki .modelf dosyasını ana Presets klasörüne geri taşıyıp Sound Library'yi yeniden açın.
- Çalışan standart Windows kaydetme ve arama pencereleri korunmuştur.

## Doğrulama

64 birim testi, 4 CLAP host testi, 1 MSEG ses testi, 3 overload testi ve 1 gerçek zamanlı bellek testi: toplam 73 test başarılı. AD, Multi, User listesi ve silme onayı 1120×760 ve 2240×1520 olarak render edildi; örnek verili görseller incelendi. Testlerde yalnız geçici preset dosyaları kullanıldı; gerçek kullanıcı presetleri silinmedi.

Windows x64 VST3, CLAP ve bağımsız EXE paketi derlendi. Bu toplu düzenlemenin Ableton içinde canlı fare/klavye testi ayrıca gereklidir. GitHub ve web yayını bu yerel derlemeye dahil değildir.