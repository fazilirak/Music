<!-- Geliştirici: Fazıl Irak | 27.10.2026 -->
# Linux x64 VST3 derlemeleri

Kapsam: F-Kick, F-Sidechain, F-TransForm, F-Saturator, F-EQ, Model-F ve Shaper.

GitHub Actions yerel Ubuntu 24.04 x86_64 üzerinde derler ve her projenin testlerini çalıştırır. JUCE GUI testleri Xvfb ve Openbox altında yapılır; pencere yöneticisinin hazır olduğu X11 kök özelliğinden doğrulanır. Rust sürümü mevcut ürünlerle aynı sabit 1.97.1 sürümüdür. ELF64 x86_64, VST3 GetPluginFactory sembolü ve eksik dinamik kütüphane olmaması ayrıca denetlenir.

Linux dosyaları Windows veya macOS dosyalarının yeniden adlandırılmış hali değildir. Linux VST3 klasörü bütünüyle ~/.vst3/ konumuna kopyalanır; yerel Linux VST3 host'unda taranır. Ubuntu 24.04 çalışma zamanı esas alınır. Diğer dağıtım, gerçek DAW, ses aygıtı ve MIDI cihazı testleri ayrıca gerekir.

Her başarılı ikiliye tam karşılık gelen uygulama/bağımlılık kaynakları, lisanslar, commit ve SHA-256 manifesti eşlik eder. Linux paketleri aynı ürün sürümünün GitHub Release sayfasına eklenir. Mevcut Windows/macOS dosyaları korunur. Başarısız derleme için ikili yayınlanmaz.

Durum: [Linux Actions](https://github.com/fazilirak/Music/actions/workflows/linux-vst3-release.yml). Paket bağlantıları yalnız başarılı yayın doğrulandıktan sonra ana sayfaya eklenir.
