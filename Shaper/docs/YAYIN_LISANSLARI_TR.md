# Shaper kaynak ve ürün dağıtımı

Fazıl Irak — 23.10.2026

Shaper kaynakları yayımlanabilir. Bütün bileşenler aynı lisansa sahip değildir;
özgün lisanslar korunur. Bu belge denetlenen yerel sürümü açıklar; kaynakların
paketlenmesi, her varlık için dağıtım izni verildiği anlamına gelmez.

| Bileşen | Yerel lisans / kayıt |
| --- | --- |
| Shaper eklemeleri ve değişiklikleri | GPLv3; COPYING.txt ve LICENSE |
| GATE-12, TIME-12, REEV-R | Kendi upstream LICENSE dosyalarında GPLv3 |
| FILT-R | upstream/filtr/LICENSE: AGPLv3 |
| Pan, Volume ve Saturation/Pitch eğri editörü | GATE-12 kaynaklarından uyarlama; özgün bildirimler korunur |
| JUCE 8.0.7 | AGPLv3 açık kaynak seçeneği; upstream/gate12/libs/JUCE/LICENSE.md |
| Signalsmith Stretch / Linear | MIT; THIRD-PARTY-NOTICES.txt |
| FFTConvolver / AudioFFT | GPL-3.0-or-later / MIT; kaynak başlıkları ve licenses/ |
| Yerel VST3 SDK sürümü | GPLv3 alternatifi; yerel SDK lisansı korunur |
| Ubuntu-Medium.ttf | Copyright 2011 Canonical Ltd.; Ubuntu Font Licence 1.0 |
| Eski Hall Quad.flac | Samplicity / Deltaworks; kaynak ve ürün dağıtımından çıkarıldı |
| Shaper_Hall_Quad.wav | Fazıl Irak; GPLv3; scripts/generate_ir.py ile bağımsız sentetik üretim |

GPLv3 ve AGPLv3 madde 13 birlikte kullanıma izin verir. Bu, bütün orijinal
kodların tek lisansa çevrilmesi değildir. JUCE için ticari lisans alınması
FILT-R ve diğer GPL/AGPL bağımlılıkların yükümlülüklerini kaldırmaz.

## Kaynak paketinin içeriği

Source/, Tests/, scripts/, CMakeLists.txt, upstream/ ve bağımlılık sürüm kayıtları
birlikte bulunur. upstream/ dosyaları değiştirilmeden korunur; Shaper uyarlamaları
üretici scriptlerde ve Source/ içinde yer alır. Ayrı bir orijinal kaynak ZIP'i
zorunlu değildir; orijinal kaynaklar zaten aynı paketin upstream/ dizinindedir.
Git geçmişi lisans uyumu için zorunlu değildir. upstream-lock.json sürüm kökenini
kaydeder; bu çalışma commit nesneleriyle birebir eşleşmeyi doğrulamamıştır.
Kaynak manifesti, pakete alınan dosyaların SHA-256 değerlerini kaydeder.

Windows derlemesi için C++17 destekli MSVC, CMake >= 3.24, Ninja, Python 3 ve
Windows SDK gerekir. `scripts/build-windows.cmd` yerel yardımcıdır; başka makinede
VSROOT ve Python yolunu uyarlayın. Daha taşınabilir komutlar:

```text
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<python-yolu>
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Komutları x64 MSVC geliştirici terminalinde çalıştırın. Kaynak paketine derleme
önbelleği veya üretilen build/modules kopyaları yerine bunları üreten scriptler
eklenir. Tüm gerekli kaynak bağımlılıklar upstream/ içinde bulunur.

## İkili dağıtımla birlikte yayımlanacaklar

EXE/VST3 ile aynı sürümün eksiksiz kaynak ZIP'ini aynı indirme yerinde, eşdeğer
erişimle sunun. Yalnızca orijinal projelerin bağlantılarını vermek yeterli değildir:
Shaper değişiklikleri ve derleme scriptleri de kaynak paketine dahil olmalıdır.
LICENSE, LICENSE-AGPL-3.0.txt, COPYING.txt, THIRD-PARTY-NOTICES.txt ve licenses/
dağıtımla birlikte tutulur. MIT/BSD/Apache/font bildirimleri silinmez.
Bu yöntem, yalnızca yazılı kaynak sağlama teklifi vermek yerine kaynakları doğrudan
sunmayı esas alır. İnternet üzerinden etkileşim sunulursa AGPL madde 13 ayrıca uygulanır.

## IR varlığı ve yapılan değişiklik

Eski `upstream/reevr/resources/impulses/Hall Quad.flac` FLAC metadata’sı:
`ARTIST=Samplicity`, `Copyright=(c) 2010-2023 Deltaworks Holding BV, The Netherlands`.
Yeniden dağıtım izni doğrulanamadığı için kullanıcı talimatıyla hem kaynak ZIP’inden
hem derlenen üründen çıkarıldı. Yerel upstream kopyası kaynak kökenini korumak için
silinmedi; yayın scripti bu dosyayı açıkça dışlar.

Yerine `Resources/impulses/Shaper_Hall_Quad.wav` kullanılır: 48 kHz, dört kanal
(LL/LR/RL/RR), iki saniye, 16-bit PCM; bağımsız sentetik gürültü sönümü ve erken
yansımalar içerir. Kayıt, örnekleme veya üçüncü taraf IR verisi kullanılmaz.
Üretici script ve SHA-256 manifesti kaynak paketindedir. Bu değişiklik varsayılan
Reverb tınısını değiştirir; eski IR’nin ölçülmüş eşdeğeri olduğu iddia edilmez.
Preset formatları ve parametre kimlikleri korunur.

Ubuntu fontunun gömülü telif kaydı incelendi ve resmi Ubuntu Font Licence 1.0
`licenses/Ubuntu-Font-Licence-1.0.txt` dosyasına eklendi. JUCE'nin alt bağımlılık
lisansları `licenses/upstream/gate12/libs/JUCE/` altında aynalanır; gömülü kaynak
bildirimleri de korunur. Bütün üçüncü taraf varlıkların kökeni için kapsamlı bir
hukuki garanti verilmez; doğrulanmamış varlıkların durumu ayrıca kayıt altındadır.

## Resmi başvuru metinleri

- [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html), özellikle 5, 6 ve 13. maddeler.
- [AGPLv3](https://www.gnu.org/licenses/agpl-3.0.html), özellikle 13. madde.
- [JUCE 8 koşulları](https://juce.com/legal/juce-8-licence/); yerel JUCE LICENSE.md açık kaynak alternatifini belirtir.
- [Ubuntu Font Licence](https://ubuntu.com/legal/font-licence).
- [Samplicity IR indirme açıklaması](https://samplicity.com/downloads/).
