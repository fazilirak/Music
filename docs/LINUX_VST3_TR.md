<!-- Geliştirici: Fazıl Irak | 28.10.2026 -->
# Linux x64 VST3 derlemeleri

Yedi ürün: F-Kick, F-Sidechain, F-TransForm, F-Saturator, F-EQ, Model-F ve Jellybone (önceki adı Shaper).

GitHub Actions, yerel Ubuntu 24.04 x86_64 üzerinde derler ve proje testlerini çalıştırır. JUCE GUI testleri Xvfb ve Openbox altında çalışır; pencere yöneticisinin hazır olduğu X11 kök özelliğinden doğrulanır. Rust sürümü 1.97.1'e sabittir. ELF64 x86_64, VST3 GetPluginFactory sembolü ve eksik dinamik kütüphane olmaması ayrıca denetlenir.

| Ürün | Linux ikilisi | Tam karşılık gelen kaynak |
| --- | --- | --- |
| F-Saturator 0.5.2 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-Saturator-v0.5.2/F-Saturator-0.5.2-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/F-Saturator-v0.5.2/F-Saturator-0.5.2-Linux-Source.tar.gz) |
| F-Kick 0.8.3 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-Kick-v0.8.3/F-Kick-0.8.3-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/F-Kick-v0.8.3/F-Kick-0.8.3-Linux-Source.tar.gz) |
| F-TransForm 0.2.2 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-TransForm-v0.2.2/F-TransForm-0.2.2-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/F-TransForm-v0.2.2/F-TransForm-0.2.2-Linux-Source.tar.gz) |
| F-Sidechain 1.1.2 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.2/F-Sidechain-1.1.2-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.2/F-Sidechain-1.1.2-Linux-Source.tar.gz) |
| Jellybone 0.8.4 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/Jellybone-v0.8.4/Jellybone-0.8.4-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/Jellybone-v0.8.4/Jellybone-0.8.4-Linux-Source.tar.gz) |
| Model-F 0.2.8 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/Model-F-v0.2.8/Model-F-0.2.8-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/Model-F-v0.2.8/Model-F-0.2.8-Linux-Source.zip) |
| F-EQ 0.1.10 | [TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-EQ-v0.1.10/F-EQ-0.1.10-Linux-x64.tar.gz) | [Kaynak](https://github.com/fazilirak/Music/releases/download/F-EQ-v0.1.10/F-EQ-0.1.10-Linux-Source.zip) |

VST3 klasörünü bütün olarak `~/.vst3/` konumuna kopyalayıp yerel Linux host'unda yeniden tarayın. Ubuntu 24.04 çalışma zamanı esas alınır; diğer dağıtım, gerçek DAW, ses aygıtı ve MIDI cihazı testleri ayrı kontrollerdir.

Her başarılı ikiliye tam uygulama/bağımlılık kaynakları, lisanslar, kaynak commit'i ve SHA-256 manifesti eşlik eder. Linux paketleri aynı ürün sürümünün GitHub Release sayfasına eklenir; mevcut Windows/macOS dosyaları korunur. Başarısız derleme için ikili yayımlanmaz.

F-Kick 0.8.2'de temiz bankayı duymak için yeni instance açın veya fabrika presetini yeniden seçin. Eski kayıtlı oturumlar kendi saturasyon ayarlarını korur.

Jellybone için kaynak klasörü, CMake hedefi ve workflow matrix anahtarı `Shaper` olarak tutulur; görünen ürün, ikili ve sürüm adı Jellybone'dur. Plugin kimlikleri ve preset/state biçimleri korunur.

[Linux derleme iş akışı](https://github.com/fazilirak/Music/actions/workflows/linux-vst3-release.yml)
