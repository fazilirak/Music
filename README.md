# Music — FAZIL AHMET IRAK

Independent music software and development resources by [FAZIL AHMET IRAK](https://fazilirak.com).

| Project | Windows | macOS | Source |
| --- | --- | --- | --- |
| [F Saturator](F-Saturator/README.md) | [0.5.1](https://github.com/fazilirak/Music/releases/tag/F-Saturator-v0.5.1) | [Universal 0.5.1](https://github.com/fazilirak/Music/releases/tag/F-Saturator-v0.5.1) | [Browse](F-Saturator) |
| [Shaper](Shaper/README.md) | [Releases](https://github.com/fazilirak/Music/releases/tag/Shaper-v0.8.2) | [Universal build](https://github.com/fazilirak/Music/actions/workflows/shaper-release.yml) | [Browse](Shaper) |
| [Model F](Model-F/README.md) | [0.2.5](https://github.com/fazilirak/Music/releases/tag/Model-F-v0.2.5) | [Universal](https://github.com/fazilirak/Music/releases/tag/Model-F-v0.2.5) | [Browse / licences](Model-F) |
| [F EQ](F-EQ/README.md) | [0.1.10](https://github.com/fazilirak/Music/releases/tag/F-EQ-v0.1.10) | [Universal](https://github.com/fazilirak/Music/releases/tag/F-EQ-v0.1.10) | [Browse / licences](F-EQ) |

## F Saturator

**Add colour. Shape the character.** Free saturation/distortion for Windows x64 and macOS Universal. VST3 and standalone, plus AU on macOS.

Five original character models — Console, Transformer, Tape, Triode and Pentode — with 16 presets, 8× FIR oversampling and two ADAA stages. Shape drive, tone, bias, dynamics and stereo width in a focused interface. Character (Warm–Sizzle), Harmonic Balance (Even–Odd) and subtle Analog Drift shape the saturation. Version 0.5.1 adds compact Transient, Comp, Bass and Treble controls with ATT/REL and Tight LED switches.

![F Saturator](F-Saturator/doc/f-saturator.png)

- [Source and build instructions](F-Saturator/README.md)
- [Türkçe geliştirme rehberi](F-Saturator/GELISTIRME-TR.md)
- [Source review notes / İnceleme notları](F-Saturator/REVIEW-TR.md)
- [Downloads / Releases](https://github.com/fazilirak/Music/releases)
- [Product overview / Ürün tanıtımı](https://fazilirak.com/f-saturator-english/)

## Türkçe

Müzik yazılımlarımı bu depoda ayrı proje klasörleriyle yayımlıyorum. F Saturator kaynaklarında Türkçe açıklamalar ve düzenleme rehberi bulunur. Windows ve macOS indirmeleri sürümlerin **Releases** bölümündedir.

## Licensing

Each project has its own licence. F Saturator is AGPLv3-or-later; see [its licence](F-Saturator/LICENSE), [JUCE notice](F-Saturator/JUCE-LICENSE.md) and embedded font notices. This repository does not assign a common licence to projects that may be added later.

Website: https://fazilirak.com · Contact: info@fazilirak.com

## Shaper

![Shaper](Shaper/doc/shaper.png)

JUCE-based effects rack: Gate, Time, Filter, Reverb, Pan, Volume, Saturation and Pitch.

- [Source, licences and Windows build instructions](Shaper/README.md)
- [macOS Universal build instructions](Shaper/docs/MACOS_TR.md)
- [macOS build workflow](https://github.com/fazilirak/Music/actions/workflows/shaper-release.yml)
- [Source and binary distribution requirements](Shaper/docs/YAYIN_LISANSLARI_TR.md)

Shaper preserves the individual GPLv3, AGPLv3, MIT and other dependency licences;
see [COPYING](Shaper/COPYING.txt) and [third-party notices](Shaper/THIRD-PARTY-NOTICES.txt).

## Model F

**Few controls. Big sound. Built especially for techno.** Circuit-modelled synthesizer: mono/poly/SuperSaw, multi-stage envelope, arpeggiator and 160 factory presets. Existing Rust/NIH-plug edition; not the cancelled JUCE rewrite.

![Model F](Model-F/docs/panel.png)

## F EQ

**Simple control. Focused tone shaping.** Tilt equalizer, independent bass shelf, steep cut filters, analog character, saturation and output spectrum. Existing Rust/NIH-plug edition.

![F EQ](F-EQ/docs/panel.png)

Both products provide Windows x64 VST3/CLAP/standalone and macOS Universal VST3/CLAP/standalone. Mac bundles are ad-hoc signed, not Developer ID signed or Apple notarised. AU is not included. Complete corresponding source accompanies each binary release. Project code is GPL-3.0-or-later; individual third-party and embedded URW Gothic AGPLv3 licences are preserved. See each project's COPYING and third-party notices.
