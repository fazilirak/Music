<!-- Geliştirici: Fazıl Irak | 27.10.2026 -->
# F-Sidechain geliştirici rehberi

F-Sidechain ürün bakımı, kompakt arayüzü ve çok bant entegrasyonu: Fazıl Irak. Volume/envelope temelinin GATE-12/Tilr kaynak atıfları korunur.

| Dosya | Görev |
|---|---|
| Source/MultibandProcessor.* | Full Range / Low / Mid / High motorları, crossover, oturum kaydı. |
| Source/Volume/PluginProcessor.* | Volume motoru ve host parametreleri. |
| Source/Volume/dsp/Pattern.* | GATE-12 kökenli eğri değerlendirme. |
| Source/Volume/utils/PatternManager.* | Eğri düzenleme ve durum yönetimi. |
| Source/AboutPanel.* | İngilizce çevrimdışı About ve yedi lisans metni. |
| Source/Volume/ui/ | Düzenleme, grid, paint, sequencer ve görsel kontroller. |
| Tests/AudioTests.cpp | Ses, state, bant ve gerçek About/lisans kontrolleri. |
| scripts/ | Sabit JUCE kaynağı, derleme, kaynak ve ikili paketleri. |

## Sözleşmeler

Parametre kimlikleri, kullanıcı eğrileri ve kayıtlı oturum davranışı korunur. Full Range varsayılandır; kullanıcı bant eklerse bağımsız eğriler işlenir. Callback içinde dosya/ağ erişimi, bellek tahsisi veya GUI işlemi eklenmez. About metinleri gömülüdür.

## Lisans ve sahiplik

Fazıl Irak katkıları GPL-3.0-only; GATE-12/Tilr özgün telifleri ve GPLv3 bildirimleri korunur. JUCE AGPLv3 ve SDK/font koşulları ayrı bileşenler olarak geçerlidir. AUTHORS.md, LICENSE.txt, THIRD-PARTY-NOTICES.txt ve licenses/ kayıtları birlikte korunmalıdır.

1.1.1 kaynak açıklamaları, sahiplik kayıtları ve yayın bakımını düzenler; yeni bir ses algoritması eklemez. Windows ve macOS paketleri ancak kendi CI testleri geçtikten sonra yayınlanır.

## Derleme ve doğrulama

Git checkout: python scripts/bootstrap_juce.py, ardından scripts/build-windows.cmd. Mac: scripts/build-macos.sh. Paketleme scripts/package_distribution.py ile eşleşen tam kaynak ve lisansları içerir. Gerçek DAW ve cihaz testleri otomatik sayısal kontrollerin kapsamını aşar.

