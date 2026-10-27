<!-- Developer: Fazıl Irak | Updated: 27.10.2026 -->
# F-Kick 0.8.1

**Sample the attack. Shape the body.**

An independently developed kick synthesizer and sampler by **Fazıl Irak**. Compact 760 × 360 interface, original DSP, editable Pitch/Amp curves and 128 original synthesis patches.

![F-Kick](preview.png)

| Downloads | Links |
| --- | --- |
| Windows x64 — VST3 + standalone | [Download ZIP](https://github.com/fazilirak/Music/releases/download/F-Kick-v0.8.1/F-Kick-0.8.1-Windows-x64.zip) |
| Complete corresponding source | [Source ZIP](https://github.com/fazilirak/Music/releases/download/F-Kick-v0.8.1/F-Kick-0.8.1-Source.zip) |
| Release notes and checksums | [F-Kick 0.8.1](https://github.com/fazilirak/Music/releases/tag/F-Kick-v0.8.1) |

## Features

- Original kick body synthesis and sample layer with trim, reverse and loop.
- Editable pitch/amplitude curves, mono/poly voices and MIDI/velocity support.
- Rumble, tempo echo, room return and four FX-only sidechain curves.
- Independent sound and FX filters, bass shelf, original factory bank and WAV export.
- Offline About and complete licence texts opened from the plain F-Kick logo.

## Install and build

Copy the entire VST3 bundle to the VST3 directory scanned by your DAW. Standalone: Standalone/F-Kick.exe. Windows binaries are unsigned. macOS targets are configured but are not part of this Windows release.

From a Git checkout run python scripts/bootstrap_juce.py, then scripts/build-windows.cmd from Windows with Visual Studio C++ tools. Complete source archives include pinned JUCE source under Dependencies/JUCE. Tests cover audio, state, GUI and VST3 loading; see validation/TEST_SONUCLARI_TR.md.

## Licence and ownership

Original F-Kick application code, DSP, interface and factory synthesis settings: **© 2026 Fazıl Irak**, AGPL-3.0-or-later. JUCE, VST3 SDK and Ubuntu font retain their respective licences. No proprietary reference-product source, samples, factory patches, branding or artwork are included. See [authors](AUTHORS.md), [licence overview](LICENSE.txt), [full AGPL](LICENSE), [third-party notices](THIRD-PARTY-NOTICES.txt) and [licence texts](licenses/).

## Türkçe kullanım ve bakım

# F-Kick 0.8.1

**Sample the attack. Shape the body.**

Fazıl Irak · Windows x64 · VST3 + Standalone · Yerel geliştirme sürümü

Sample + sentezlenen kick gövdesi, çizilebilir Pitch/Amp eğrileri, 128 özgün preset.
Ubuntu fontlu 760 × 360 koyu/sarı panel; ekran klavyesi ve knob bankası yoktur.

## Başlangıç

`dist/F-Kick-0.8.1-Windows-x64/Standalone/F-Kick.exe` bağımsız uygulamadır.
Audio/MIDI Settings ile çıkış/MIDI cihazını seçin; üst üçgeni basılı tutarak ön dinleyin.
DAW için paketteki `VST3/F-Kick.vst3` klasörünün tamamını taranan VST3 konumuna kopyalayıp taramayı yenileyin.
Yerel sistem kurulumu otomatik yapılmaz. GitHub Actions testleri geçen paketleri Releases bölümünde yayınlar.

## Arayüz

**Sound / FX / SC** ana sayfalardır. Sağdaki sütun bütün sayfalarda **Room Mix → HP / LP → Bass Lift / Q → Tempo** gösterir.
Grafik 178 piksel yüksekliğe büyütüldü. Sound ve FX sayfalarında Gain/Tune/Pan/Start/End ile Loop/Reverse
sample ekranının içinde tek satırdır. Sayısal alanlar sürükleme, tekerlek ve çift tıkla giriş destekler.
Shift ince ayar; Alt-tık varsayılan; filtrelerde 5k, yüzde alanlarında 50% yazılabilir.
Logo normal yazıdır; tıklamak About/lisansları açar. Preset okları seçicinin iki yanındadır.

## Sample ve gövde

- Dosya adına tıklayın veya ses dosyasını sample alanına bırakın. Mono/stereo, en fazla 30 saniye ve
  8–192 kHz; temel WAV/AIFF/FLAC. Yükleme arka plandadır. Gerçek Explorer→Ableton fare doğrulaması açık kontroldür.
- **Kick:** Amp/Length tek vuruşu belirler. Gate açıksa note-off Release uygular.
  **Sampler:** Attack, Length ile ölçeklenen decay, Sustain/Release; sample ve Body notaları izler.
- Start/End ile kırpın; dış waveform bölgeleri hafif karartılır. Loop Sampler'da basılı nota boyunca
  seçili aralığı tekrarlar. Reverse ters çalar. Sample Gain Off ise yeni dosya yüklemek Gain'i açar.
- Pitch/Amp noktalarını sürükleyin; boş yere çift tık ekler, iç noktaya sağ tık siler.
  Alt-sürükleme segment eğriliğini değiştirir. En fazla sekiz nokta; ilk/son zaman sabittir.
- Body: Level, Root, Sweep, Phase, Harmonics, Punch. Punch kısa harmonik ve gövde yoğunluğu ekler.
  Yeni kick'lerde Track tail kapalıdır; root MIDI ile aşırı pesleşmez. Referans MIDI60 / Ableton C3.
- Poly 16 ses, kapalıyken mono retrigger. Velocity, CC64, ±2 semiton pitch wheel, CC120/123 desteklenir.
- Sound Filter/Output: LP, Q, bağımsız HP, Drive ve Output. Son peak clip lookahead limiter değildir;
  Drive/Punch oversampling içermez. Kick sonunda 3 ms ve WAV sonunda kısa fade vardır.

## FX / Room Mix

Rumble: Amount/Decay/HP/LP/Drive/Duck/Recover/Gain. Echo: Amount/Time/Feedback/LP, Kick/Rumble kaynak,
Sync açıkken nota süreleri, kapalıyken ms. Soft/Clip/Fold ana ve rumble Drive için ortak karakterdir.

**Room Mix 0–100%**, kısa stereo odanın paralel dönüş miktarıdır. 0% kapalı; 100% eski Room On seviyesidir.
Oda boyutu/damping/genişliği sabittir. Sağdaki FX HP/LP, rumble + echo + room ortak dönüşünü filtreler.
HP Off=0 Hz, LP Off=20 kHz bypass. Sound HP bütün çıkışa, Rumble HP/LP yalnız rumble koluna aittir.
Kuru kick/sample bu sağ filtrelerden ve SC'den geçmez.

## SC

**Pump / Smooth / Late / Gate** dört eğri. Mix 0% bypass, 100% tam eğri. Grafiği yatay sürükleyin;
Shift bir vuruşun −%50…+%50'sidir. Pozitif sağa geciktirir, çift tık sıfırlar. 1/4 nota döngüsü DAW çalarken
host PPQ'ya kilitlenir; transport yokken MIDI nota yeniden tetikler. Tempo DAW'dan veya sağdaki Tempo'dan alınır.
Duck ile beraber kuyruk çok kısılırsa Mix/Duck azaltılır. SC dışında LFO/AD düzenleme görünümü ve menüsü yoktur;
eski projelerin ve Wobble presetlerinin programlanmış hareketleri DSP'de korunur.

## Preset ve export

**64 General / 48 Techno / 16 Experimental**; tam liste/kaynaklar docs/PRESET_KATALOGU_TR.md.
Sampler init, Undo/Redo ve reset komutları ... menüsündedir. Factory seçimi sample dosyasını korur,
Gain'i Off yaparak dosyadan bağımsız çalar; Gain elle açılıp yeniden katmanlanabilir.
.fkick ve DAW state'i bütün parametre/eğrileri ve gömülü sample'ı taşır. Eski Room 0/1 → Mix 0%/100%,
aynı ID/indeks ve aynı ses. 0.5 öncesi sesler yeni Punch/Gain'i nötr açar; yeni factory sesini duymak için
preset değiştirip geri dönün veya Reset current factory patch seçin.

WAV: MIDI60, velocity110, 48 kHz stereo 24-bit, hesaplanan FX kuyruğuyla. validation/ içinde 128 portable
preset, kategori/SC örnekleri ve C3/069 eşit-peak A/B dosyaları bulunur. Ölçümler tüm mixlerde beğeni garantisi değildir.

## Derleme

VS C++ Desktop + Windows SDK + CMake 3.24+. scripts/build-windows.cmd Release VST3/Standalone ve CTest çalıştırır.
Kaynak ZIP JUCE içerir; workspace alternatifi ../_Dependencies/JUCE veya CMake JUCE_PATH.
Geliştirici rehberi docs/, test kapsamı validation/ içindedir. macOS derlenmedi; Warp/slicing/otomatik kick analizi yoktur.
F-Kick AGPL-3.0-or-later; JUCE/font/SDK koşulları LICENSE.txt, THIRD-PARTY-NOTICES.txt ve licenses/ içinde korunur.

## Bass EQ — 0.8.0

F-EQ projesindeki Bass Lift davranışı: sabit 300 Hz low shelf, −12…+12 dB.
Q 0.1…2.0 aralığında shelf geçişini şekillendirir; varsayılan 0.707.
Kick, sample ve FX toplamına uygulanır; çıkış seviyesi/peak korumasından öncedir.
0 dB tam bypass: eski preset ve kayıtların sesini korur. 20 ms parametre rampası vardır.
HP/LP yalnız FX dönüşlerine etki etmeye devam eder; Bass Lift/Q onlardan bağımsızdır.
Sağ sütunda Room Mix, altında yan yana HP/LP, onun altında Bass Lift/Q bulunur.
