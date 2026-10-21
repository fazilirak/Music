# F Saturator 0.2.0 — FAZIL IRAK

JUCE tabanlı, bağımsız **VST3 ve standalone saturation/distortion** projesi. Aynı DSP motoru Shaper içinde **Saturation** efekti olarak da kullanılır.

## Kullanım

- **Console, Transformer, Tape, Triode, Pentode**: beş özgün renk modeli.
- **Drive** 0–36 dB; **Heat** ek +20 dB. **Auto Gain** referans tona göre Drive seviyesini dengeler; Output ile son seviyeyi ayarlayın.
- **Tone** parlak/koyu tilt; **Low cut** distortion öncesi, **High cut** distortion sonrası filtre.
- **Bias** yarım dalga asimetrisi; **Dynamics** dinamik yükleme/sag miktarı.
- **Mix** paralel dry/wet; **Stereo** wet stereo genişliği; **Bypass** gecikmesi korunmuş geçiş.
- Sabit **8x FIR oversampling + iki ADAA katı**, örnekleme hızına bağlı yumuşatmalar ve DC temizleme.
- 16 özgün hazır preset; önceki/sonraki ve `.irakpreset` kaydet/yükle.
- Shaper'da grafik Low/Mid/High band kontrolü, bağımsız üç motor, ortak sağ üst Mix/Stereo/dB paneli ve rack presetleriyle tam durum saklama.

Modeller analog davranışlardan esinlenen hibrit tasarımlardır. Belirli bir cihazın ölçülmüş kopyası veya Decapitator ile aynı ses oldukları iddia edilmez. Tape karakteri tape hızını, hiss veya wow/flutter'ı modellemez.

## Derleme

Windows: `scripts/build-windows.cmd`. Visual Studio 2026 C++ araçları ve CMake/Ninja kullanılır.

JUCE yolu varsayılan olarak `../upstream/gate12/libs/JUCE`'dur. Proje ayrı bir konuma taşınırsa CMake'e `-DIRAK_JUCE_PATH=C:/path/to/JUCE` verin. Proje başka bir Shaper kaynak dosyasına ihtiyaç duymaz.

Araştırma depoları derlemeye katılmaz. Ayrıntılı kaynak incelemesi, lisans notları, DSP denklemleri ve tasarım sınırları: [DSP-RESEARCH.md](Research/DSP-RESEARCH.md). Otomatik ölçüm raporu: `dist/DSP-REPORT.md`.

## Lisans

F Saturator özgün kaynak kodu: Copyright 2026 Fazıl Irak, AGPLv3-or-later. JUCE, AGPLv3 / ticari lisans seçenekleriyle ayrıca lisanslıdır. Açık kaynak dağıtımda F Saturator kaynakları ve kullanılan JUCE sürümü erişilebilir olmalıdır. Referans projelerin kodu veya medya dosyaları ürüne dahil edilmemiştir.

## F EQ arayüz standardı

Koyu gri #141819 panel, #ed6c46 turuncu dış halka, #e5e7df kırık beyaz orta halka ve #292f31 gri yivli iç kapak kullanılır. URW Gothic Book/Demi fontları ürüne gömülüdür; lisans ve kaynak bilgileri Resources/fonts içindedir.

Üst sırada saturation transfer eğrisi, Heat / Auto Gain / Bypass checkbox'ları ve dikey IN / OUT seviye göstergeleri bulunur. Ortada üç bağımsız konsantrik kontrol grubu vardır. İçten dışa: Tone / Low Cut / High Cut; Drive / Bias / Dynamics; Mix / Output / Stereo. Model düğmeleri altta, orta grubun merkezine göre hizalıdır.

Hedef halkayı yukarı/aşağı sürükleyin veya tekerleği kullanın. Shift hassas ayar; çift tıklama veya Ctrl/Alt + tıklama varsayılan değere dönüş sağlar. Alt sayısal değerlere çift tıklayarak doğrudan değer yazabilirsiniz. Low Cut / High Cut halkalarının görsel frekans ölçeği logaritmiktir; mevcut host normalizasyonu değişmez.

Bu proje F Saturator klasöründe bağımsız CMake projesidir. Yalnız JUCE bağımlılığı dışarıdan seçilir (IRAK_JUCE_PATH). Shaper dosyalarına derleme bağımlılığı yoktur; varsayılan JUCE yolu bu bilgisayardaki Shaper deposunu gösterir. Plugin kimlikleri, parametre ID'leri, IRAK_STATE ve .irakpreset biçimi önceki IRAK oturumları/presetleri için korunur. Görünen ürün adı F Saturator'dır.

## Kaynakları düzenleme ve GitHub

Türkçe açıklamalar `Source`, `Tests`, CMake ve betik dosyaları içindedir. Dosya haritası, sinyal zinciri, parametre/preset düzenleme ve derleme adımları: **[GELISTIRME-TR.md](GELISTIRME-TR.md)**. İnceleme kapsamı ve bakım notları: **[REVIEW-TR.md](REVIEW-TR.md)**.

Depo kaynakları içerir; Windows indirme paketleri sürüm etiketiyle GitHub Releases bölümünde paylaşılabilir. `build` ve `dist` üretilmiş çıktılardır, depoya eklenmez. Bu proje Windows x64 VST3 ve standalone üretir; CLAP hedefi bulunmaz.

## Building from source (English)

F Saturator uses C++17, CMake 3.24+, Ninja and the Visual Studio desktop C++ tools. Obtain JUCE separately from https://github.com/juce-framework/JUCE and check out commit `10a589619b452c261b2940767eb253171eb5a823`, recorded in `Research/dependencies.json`. From a Visual Studio x64 Native Tools terminal:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DIRAK_JUCE_PATH=C:/path/to/JUCE
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

The VST3 and standalone binaries are under `build/IRAK_artefacts/Release`. Tests generate the presets, screenshots, original audition WAVs and DSP report under `dist`. After successful tests, run `powershell -ExecutionPolicy Bypass -File scripts/package.ps1` to create the Windows and corresponding-source ZIPs. The local build helper assumes Visual Studio 2026 Community; adjust its `VSROOT` for a different installation.

F Saturator is licensed under AGPLv3-or-later; see `LICENSE`. JUCE has its own AGPL/commercial terms in `JUCE-LICENSE.md`. Embedded URW Gothic font notices and the font exception are retained in `Resources/fonts`. Research-only repository references are documented, not compiled into the product. Keep the matching source, dependency revision and build instructions available with each binary release.
