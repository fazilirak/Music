<!-- Geliştirici ve bakım: Fazıl Irak | 27.10.2026 | GATE-12/Tilr atıfları korunur -->
# F-Sidechain 1.1.1

**Draw the groove. Make room for your sound.**

Free, open-source multiband volume shaping by **Fazıl Irak**. A compact 760 × 426 interface, eight editable envelope presets and independent Low/Mid/High curves. Start in Full Range, add frequency bands only when needed, and synchronise movement to your DAW or use MIDI/audio triggers.

![F-Sidechain](preview.png)

| Downloads | Links |
| --- | --- |
| Windows x64 — VST3 + standalone | [Download ZIP](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.1/F-Sidechain-1.1.1-Windows-x64.zip) |
| Linux x64 — VST3 | [Download TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.1/F-Sidechain-1.1.1-Linux-x64.tar.gz) |
| macOS Universal — VST3 + AU + standalone | [Download TAR.GZ](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.1/F-Sidechain-1.1.1-macOS-Universal.tar.gz) |
| Complete corresponding source | [Source ZIP](https://github.com/fazilirak/Music/releases/download/F-Sidechain-v1.1.1/F-Sidechain-1.1.1-Source.zip) |
| Release notes and checksums | [F-Sidechain 1.1.1](https://github.com/fazilirak/Music/releases/tag/F-Sidechain-v1.1.1) |
| Website | [Türkçe](https://fazilirak.com/f-sidechain/) · [English](https://fazilirak.com/f-sidechain-english/) |

## Features

- Eight factory shapes, editable points, curvature and segment types.
- Full-range processing or independent Low/Mid/High bands with a visible crossover graph.
- Tempo-synchronised, MIDI and audio triggering; external sidechain input when routed by the host.
- Paint and sequencer editing, grid/snap, undo/redo, reverse, invert, double and curve copy/paste.
- Per-band Mix, output gain and stereo phase controls.
- Preset selection, Save, Load and options together in the top bar. Save/Load stores all bands and envelopes; DAW sessions retain the full state.
- Offline About/licence panel opened from the logo.

## Install and build

Windows: extract the archive, copy the **entire** `VST3/F-Sidechain.vst3` bundle into your shared VST3 folder, and rescan in the DAW. Standalone: `Standalone/F-Sidechain.exe`.

macOS: copy the VST3 bundle to `~/Library/Audio/Plug-Ins/VST3/`, the AU to `~/Library/Audio/Plug-Ins/Components/`, and open the app for standalone use. macOS 11.0 or later. Universal Intel + Apple Silicon. Bundles are ad-hoc signed, not Developer ID signed or Apple notarised. Separate real DAW and Intel Mac testing remains necessary.

From a Git checkout, run `python scripts/bootstrap_juce.py` to fetch the pinned JUCE revision. A complete source ZIP already includes `Dependencies/JUCE`. Build with `scripts/build-windows.cmd` or `bash scripts/build-macos.sh`. CTest checks audio, presets/state and embedded licences.

## Licence and credits

F-Sidechain is independently maintained from the Shaper Volume / GATE-12 foundation. It does not require Shaper at runtime. GATE-12 © Tilr 2025; F-Sidechain/Shaper modifications © Fazıl Irak 2026. Preserve all component licences. Project changes are GPL-3.0-only and the combined distribution preserves the applicable JUCE AGPLv3 terms. See [licence](LICENSE.txt), [third-party notices](THIRD-PARTY-NOTICES.txt) and [full licence texts](licenses/). Complete corresponding sources accompany the binaries.

## Türkçe kullanım ve bakım


Fazıl Irak · Windows x64 · VST3 + Standalone

Shaper'ın Volume motorundan ayrılmış, 760 × 426 boyutunda kompakt volume-shaping efekti. Shaper projesini değiştirmez; çalışırken Shaper gerektirmez.

## Kullanım

- Tempo menüsünden envelope uzunluğunu seçin. Sync DAW temposunu izler; MIDI ve Audio tetikleme seçenekleri de korunmuştur.
- Sekiz küçük eğri düğmesi Volume presetlerini yükler. Her presetin en alt noktası tam tabana (sıfır ses seviyesine) iner; üst noktaları korunmuştur. Üst başlıkta, VOLUME SHAPER yazısının sağındaki preset menüsü aynı bankayı isimle seçer. Hazır şekil/preset seçimi seçili banda uygulanır.
- Envelope üzerinde noktaları, eğriliği ve segment tiplerini düzenleyin. Alt+click nokta ekler; nokta üzerinde sağ tık segment tipini açar. Shift ince ayar/snap davranışı sağlar.
- Paint, Seq, Curve, Grid, Snap, undo/redo ve nudge kontrolleri özgün motoru kullanır. Pattern seçimi kaldırılmıştır.
- Bant grafiği sürekli açıktır. Yeni instance Full Range olarak başlar: Mid motoru tüm frekansları bölünmeden işler. Sol/sağ sınırı sürükleyerek Low/High bantlarını ekleyin; sekmelerden bağımsız eğri/preset/mix/çıkış ayarlarını düzenleyin. Full range düğmesi ayırma sınırlarını 20 Hz / 20 kHz'e döndürür ve Mid'i seçer. Bands görünürlük düğmesi yoktur.
- Sağ üstte Mix, çıkış dB/metre ve stereo faz farkı vardır. Değerleri sürükleyin veya fare tekerleğini kullanın; çift tık varsayılana döndürür.
- Üst başlıktaki preset seçimi, Save, Load ve `...` araçları tek satırda yer alır; ayrı alt araç satırı yoktur. Envelope alanının yüksekliği korunarak pencere 34 piksel kısaltılmıştır.
- Save/Load bütün bantları, eğrileri, sequencer'ı ve parametreleri `.fsidechain` dosyasında saklar. DAW oturum kaydı aynı durumu içerir. Önceden kaydedilen kullanıcı eğrilerine otomatik derinlik değişikliği uygulanmaz.
- `...` menüsünde eğri copy/paste, invert, reverse, double, clear, MIDI kanalı ve anti-click seçenekleri bulunur.
- F-Sidechain logosuna tıklamak geliştirici, kaynaklar, sürüm ve yedi gömülü lisans/bildirim metnini açar. Ayrı About/Hakkında düğmesi yoktur. Sayfanın kullanıcı metinleri İngilizcedir; çevrimdışı çalışır. Close ile editöre dönülür.

Ekran görüntüsünde işaretlenen 12'li düğme sırası ve Rate/Phase/Attack/Release/Tension/Min/Max kontrol bloğu ana arayüzden kaldırıldı. Bu motor parametreleri preset ve host automation uyumluluğu için içeride korunur. Görsel eğri düzenleme değişmemiştir.

## Kurulum

`F-Sidechain.vst3` klasörünün tamamını DAW'ın taradığı bir VST3 klasörüne kopyalayın (ör. `C:\Program Files\Common Files\VST3`). DAW'da plugin taramasını yenileyin. Paket otomatik kurulum yapmaz.

`F-Sidechain.exe` bağımsız uygulamadır. Ses cihazını uygulamanın audio settings bölümünden seçin. Standalone bir DAW'ın tempo bilgisini alamaz; host sync için VST3 sürümünü kullanın. Stereo giriş/çıkış desteklenir; dış kick tetiklemesi için DAW'ın sidechain girişini kullanın ve `...` menüsünden external sidechain'i seçin.

## Derleme ve doğrulama

Visual Studio C++ toolchain, CMake 3.24+ ve JUCE gerekir. Dağıtımdaki Source ZIP'i sabitlenmiş JUCE kaynağını `Dependencies/JUCE` içinde içerir; çalışma alanında `../_Dependencies/JUCE` kullanılır. Başka konumda `cmake -S . -B build -DJUCE_PATH="JUCE/yolu"` ile belirtilebilir. Windows yerel derlemesi: `scripts/build-windows.cmd`.

`ctest --test-dir build --output-on-failure` ses/state ve lisans sayfası testlerini çalıştırır. Test uygulamasına `--snapshots` verilmesi ana editör ve About önizlemelerini üretir. Otomatik testler gerçek bir DAW'da canlı kullanım veya bütün hostlarla uyumluluk doğrulaması değildir.

## Kaynak haritası ve köken

`Source/Volume`: Shaper `build/modules/volumef` üretiminin 26.10.2026 tarihinde alınmış ve bu proje için bağımsız olarak bakımı yapılan kaynak anlık görüntüsü. Temel GATE-12 motoru, envelope/paint/sequence altyapısı ve Shaper Volume presetleri korunmuştur. Artık Shaper'ın üretilmiş dosyaları üzerinde çalışılmaz; değişiklikler bu projedeki kaynaklarda yapılır.

`Source/MultibandProcessor.*`, `BandGraph.h`, `EffectOutputPanel.h`, `ForwardParameter.h`: Shaper'dan alınmış bant/state/automation altyapısı. Compact editör MultibandProcessor.cpp içindedir. `Source/Entry.cpp` yalnız Volume motorunun üç bağımsız kopyasını oluşturur. Diğer Shaper efektleri bu ürüne bağlanmaz.

`Source/Embedded/VolumePresetData.h`: Shaper'dan türetilmiş sekiz Volume presetinin XML verisi; 1.1.0 sürümünde alt noktalar tabana uzatıldı. `scripts/deepen_factory_presets.py` aynı dönüşümü tekrar güvenle uygulayabilir. `Tests/AudioTests.cpp`: DSP, preset/state ve editör doğrulaması. `Source/AboutPanel.*`: çevrimdışı, gömülü lisans metinleriyle About görünümü. Eski state ve sequencer uyumluluğu için iç desen depolaması ve eski parametre kimlikleri korunur; pattern seçici arayüzde ve seçenekler menüsünde bulunmaz.

GATE-12 © Tilr 2025; Shaper uyarlamaları ve F-Sidechain © Fazıl Irak 2026. Lisanslar `licenses/` altında korunmuştur. Bu ürün Kickstart 2 kodu veya varlığı içermez; arayüzde yalnız kullanım sadeliği referans alınmıştır.

Lisans seçimi: F-Sidechain/Shaper kaynakları GPL-3.0-only; GATE-12 GPLv3; JUCE'nin bu derlemesi AGPLv3 açık kaynak seçeneği; VST3 SDK GPLv3 seçeneği; yazı tipi Ubuntu Font Licence 1.0. Birleşik dağıtım GPLv3 ve AGPLv3'ün ilgili hükümlerini korur. Ayrıntı ve tam metinler `LICENSE.txt`, `THIRD-PARTY-NOTICES.txt` ve `licenses/` içindedir. Kaynak ZIP'i uygulama, derleme betikleri, kaynak varlıklar ve kullanılan JUCE/VST3 kaynaklarını içerir.
