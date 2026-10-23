# IRAK — DSP araştırması ve tasarım kararları

İnceleme: 20 Ekim 2026. Araştırma kopyaları `repositories/` altında; ürün koduna bağlanmaz.

## İncelenen kaynak kod

**[Alain Paul / AP Saturation](https://github.com/apmastering/Saturation)** — commit `c9145ac3494d98e2227510777357cfc743cc6608`. BSD 3-Clause biçimindeki lisans ve marka onayı yasağı yerel LICENSE dosyasında incelendi. `Source/PluginProcessor.cpp` içinde `processBlock`, `startOversampler` ve sekiz `do...` şekillendiricisi okundu. Tanh, sine, yumuşatılmış sert kırpma, log/sqrt/cube, fold ve squared-sine farklı harmonik aileler üretir. İşlem 8x FIR oversampling içinde yapılır. Kontrol yumuşatmasını kanal döngüsü içinde güncellemekten kaçındık: IRAK ortak parametreleri her yüksek hızlı örnekte bir kez ilerletir. Fold ve sqrt gibi süreksiz türevli seçenekler bu analog odaklı ilk sürüme alınmadı.

**[Resonant DSP / SwankyAmp](https://github.com/resonantdsp/SwankyAmp)** — güncel commit `f9b206a074b1c87ba23137d223586ab5feeaefed`; ayrıca `juce-1.4.0` etiketi. GPLv3 lisansı incelendi. Güncel `src/dsp/stage.rs`, `amp.rs`, `filters.rs`, `oversample.rs` ve eski `Source/dsp/Triode.h`, `PushPullAmp.h` incelendi. Grid iletimi/yük birikimi, plate saturation, bias drift, push-pull katı, sag, örnekleme hızına bağlı zaman sabitleri ve seviye kalibrasyonu ayrı sorumluluklar olarak ele alınıyor. Gitar amplifikatörüne özgü ton yığını, kabin ve ayarlanmış katsayılar IRAK'a taşınmadı. Tam miks kullanımı için kabin filtresi olmadan daha geniş bantta çalışan özgün bir zincir tasarlandı.

Bu depolardan kaynak kod, arayüz varlığı, font, preset veya model katsayısı kopyalanmadı. Referanslar öğrenme ve karşılaştırma içindir. IRAK kaynakları AGPLv3-or-later ile sunulur; JUCE ayrıca kendi lisansına tabidir.

## Birincil araştırma kaynakları

1. [Parker, Zavalishin, Le Bivic — Reducing the Aliasing of Nonlinear Waveshaping Using Continuous-Time Convolution, DAFx 2016](https://www.dafx.de/paper-archive/details.php?id=vem_XXF5qBbfiWOH2RVVAA). Antitürev fark oranıyla örnekler arasındaki doğrusal giriş yaklaşımının integralini almak, statik nonlineerlikte aliasing'i azaltır. IRAK'ta iki tanh tabanlı kat için birinci dereceden ADAA kullanılır. Fark küçükse sayısal iptali önlemek için orta noktadaki fonksiyon değerine geçilir. Antitürev double precision ve taşmayan log-cosh ile hesaplanır. Dinamik manyetik kolun aynı analitik varsayıma uymadığı dikkate alınarak bütün zincir ayrıca 8x örneklenir.

2. [Jatin Chowdhury — Real-time Physical Modelling for Analog Tape Machines, DAFx 2019](https://www.dafx.de/paper-archive/2019/DAFx2019_paper_3.pdf). Kayıt kafası, manyetizasyon ve okuma kafası ayrı süreçlerdir. Jiles–Atherton yaklaşımında çıkış yalnızca anlık girişe bağlı değildir; manyetik alanın geçmişi önemlidir. IRAK normalleştirilmiş, sınırlı alan adımlarıyla midpoint yöntemi kullanan bir manyetik kol oluşturur. Bu kol renk katına düşük oranda karıştırılır. Tam kafa geometrisi, tape hızı, taşıyıcı bias, wow/flutter ve hiss modellenmez; bu nedenle Sony veya başka bir belirli tape makinesinin fiziksel kopyası olarak sunulmaz.

3. [Holters, Zölzer — Circuit Simulation with Inductors and Transformers Based on the Jiles-Atherton Model of Magnetization, DAFx 2016](https://www.dafx.de/paper-archive/2016/dafxpapers/08-DAFx-16_paper_10-PN.pdf). Manyetik hafızanın transformatör/devre modeline katılabilmesi incelendi. IRAK Transformer modelinde alanı besleyen alçak frekans kolu bu yaklaşımdan esinlenir. Devre şeması çözümü, sarım geometrisi veya belirli bir donanımdan ölçülmüş değerler kullanılmaz. Model, devre eşdeğeri olduğunu iddia etmeyen bir davranış modelidir.

4. [Soundtoys Decapitator — resmî kullanım kılavuzu](https://www.soundtoys.com/wp-content/uploads/Decapitator-Manual.pdf). Farklı karakterleri az sayıda kontrolle seçme, ilave drive, ton biçimlendirme ve paralel Mix iş akışı incelendi. IRAK'ın beş karakteri kendi adlarına, kendi denklemlerine ve kendi presetlerine sahiptir. Soundtoys'un kodu, donanım ölçümleri, presetleri, A/E/N/T/P eşlemeleri ve görsel tasarımı kullanılmaz. Decapitator ile ses eşdeğerliği doğrulanmış değildir.

## IRAK sinyal zinciri

Giriş → 8x lineer fazlı FIR interpolasyon → isteğe bağlı pre low-cut → Drive/Heat → paylaşılan envelope ile dinamik yükleme → seçili model → iki ADAA katı → Tone tilt → isteğe bağlı high-cut → DC temizleme → FIR desimasyon → kesirli gecikme düzeltmesi → Auto Gain → stereo genişliği → gecikmesi eşlenmiş paralel Mix → Output → gecikmesi eşlenmiş bypass.

Tüm ses belleği `prepare()` içinde ayrılır. Büyük host blokları önceden ayrılmış parçalarda işlenir. Tüm modeller aynı oversampling oranıyla çalışır; otomasyon için belleksiz parametreler 25 ms, karakter ağırlıkları 20 ms yumuşatılır. Model değişiminde eski ve yeni katlar kısa süre birlikte işlenir; sıfır ağırlıklı bir model tekrar seçilirse yalnızca küçük sabit durum yapısı sıfırlanır. Stereo kanalların DSP geçmişi ayrıdır; sag zarfı ortak güç kaynağı davranışını temsil etmek üzere paylaşılır.

## Beş karakter

| Karakter | Tasarım |
|---|---|
| Console | Düşük asimetri, yumuşak iki kat; hafif dinamik yükleme. |
| Transformer | Daha sıkı knee ve düşük frekansla beslenen manyetik hafıza kolu. |
| Tape | Kayıt/okuma pre-emphasis/de-emphasis çifti, düşük oranda manyetik hafıza, daha yumuşak knee. |
| Triode | Asimetrik iki kat, pozitif grid yükünden kaynaklanan yavaş çalışma noktası değişimi. |
| Pentode | Daha sert, baskın olarak simetrik iki kat; daha güçlü dinamik yükleme. |

Bias, pozitif/negatif yarı dalga asimetrisini düzenler; tape bias taşıyıcısı değildir. Dynamics, giriş zarfının katların yüklenmesine etkisidir. Heat, nonlinear kat öncesinde +20 dB ekler. Auto Gain sabit referans tonundan türetilen Drive telafi eğrisidir; her kaynakta eşit LUFS veya tepe garantisi veren bir otomatik seviyeleyici değildir.

## Doğrulama

`Tests/DSPTests.cpp`: ham 1x işleme karşı spektral alias enerjisi; 44.1/48/96/192 kHz; azami Drive/Heat/Bias/Dynamics; sessiz kuyruk; model geçişleri; gecikmesi eşlenmiş dry/bypass; büyük blok; 16 preset geri yükleme; arayüz görüntüsü. Kalibrasyon `IRAKTests.exe --calibrate` ile üretilir. Ölçülmüş sonuçlar `dist/DSP-REPORT.md` içindedir.

Ölçümler sayısal davranışı sınar. Donanım veya ticari plugin ile dinleme eşdeğerliği kanıtı değildir; nihai tonal tercih gerçek kayıtlarla dinlenerek değerlendirilmelidir.
