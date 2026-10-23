# Shaper — Türkçe geliştirici ve hata müdahale rehberi

**Fazıl Irak · Son düzenleme: 22.10.2026**

Shaper 0.8.1 rack efekt projesi. Ses motoru, modül kaynak üretimi, standart envelope ve ortak Low/Mid/High/Stereo/output/Mix yerleşimi korunmuştur.

## Derleme ve bağımlılıklar

Windows C++/CMake/Ninja ve Python 3 gerekir. Yerel build-windows.cmd Python yolunu içerir; başka makinede Python3_EXECUTABLE değerini uyarlayın. upstream kaynakları kendi klasörü içinde bulunur. .git olmayan temiz kopyada commit kökeni upstream-lock.json’dan okunur; bu kayıt kaynak bütünlüğünün yerine geçen imza değildir.

## Veri ve ses akışı

Host → Processor → sıralı efekt zinciri → MultibandProcessor (Low/Mid/High bağımsız motorları) → EnvelopeEffect veya gömülü motor → ortak çıkış. Generator `scripts/prepare_sources.py` upstream kaynakları build/modules altına uyarlar. Bu üretilmiş dosyaları veya upstream repoları değil üreticiyi düzenleyin.

## Değişiklik ve hata dosyaları

UI: Source/Editor.cpp, BandGraph.h, EffectOutputPanel.h. Rack kaydı/parametre: Processor.cpp, ForwardParameter.h. Bant bölme/karışım: MultibandProcessor.cpp. Saturation/Pitch eğri yolları: EnvelopeEffect.cpp ve EnvelopeEditor.cpp; kaynak motorlar Source/Saturation ve Source/Pitch. Yeni rotary görünümü tasarlamayın: AGENTS.md ve Embedded/RackKnob.h standardı geçerlidir.

## Belirtiyi yorumlama

Bir efekt ses vermiyorsa önce rack enabled/bypass, sıra ve ortak Mix; sonra band aktifliği/crossover ve ilgili alt motoru kontrol edin. State sorunu için efekt kimliği, sıra ve iç state XML’ini izleyin. Pitch/formant hatasında eğrilerin ayrı kayıtlarını ve latency/reset yolunu ayırın. UI düzeltmesi sırasında audio davranışını değiştirmeyin.

## Testler

`scripts/build-windows.cmd` tüm mevcut CTest hedeflerini derler/çalıştırır: audio_integration, irak_dsp, pitch_dsp ve curve_effects. Testler sentetik ses, preset/geri çağırma, band/crossover, envelope/tetik ve UI sözleşmelerini kontrol eder. build/Testing/Temporary/LastTest.log ilk hata mesajını taşır.

## Adım adım hata ayıklama

1. Host/ürün/sürüm, örnekleme hızı, blok boyu, mono/stereo, preset ve hareketi kaydedin. Orijinal DAW projesinin kopyasını kullanın. Standalone ile tekrarlayın; yalnız belirli host’ta oluyorsa host testini ayrıca çalıştırın.
2. Visual Studio/VS Code debugger ile önce host callback veya UI olayına, sonra parametre dönüşümüne, sonra ilgili motor fonksiyonuna kesme noktası koyun. Call Stack çağrı yolunu, Watch/Locals gerçek değer ve birimi gösterir. Audio callback’te debugger beklemesi dropout yaratabilir; bunu tek başına DSP hatası saymayın.
3. Parametre id/index, 0..1 normalized değer ile dB/Hz/saniye/semiton fiziksel değeri ayırın. Eşik/örnekleme/tempo değişikliği tüm türetilen katsayı ve gecikmeleri etkileyebilir. Grafik ile gerçek motor aynı ölçüm değildir.
4. Geçmiş filtre/voice/delay durumunu, reset’i, kapalı pencereyi ve ses callback’inin GUI’den bağımsız çalışmasını kontrol edin. Unsafe/FFI için null, buffer uzunluğu, pointer sahibi ve thread ömrünü birlikte izleyin.
5. Gerçek zamanlı ses yoluna dosya/preset okuma, ağ, mesaj kutusu veya yeni bekleyen mutex eklemeyin. Arızayı sentetik offline testte tekrarlamak çoğu zaman daha kolaydır. UI telemetry için mevcut atomik/paylaşımlı sözleşmeyi kullanın.
6. En küçük düzeltmeyi yapıp ilgili normal/sınır/boş veri testini çalıştırın. Başarısız testi susturmak için sabit eşiği değiştirmeyin; beklenen davranış gerçekten değişiyorsa bunu yeni sürüm ve preset/state geçişiyle belgeleyin.

## Yeni parametre veya özellik ekleme

Host id/aralık/varsayılan → gerçek zamanlı kontrol modeli → rampalar/katsayı ve örnek kullanımı → GUI binding/hit-test → kaydet/yükle/preset geçişi → regresyon. Yeni özellikte bu zincirin tamamını kontrol edin. Mevcut ürün/host id, state kökü, program indeksleri ve MIDI eşlemesi eski kayıt sözleşmesidir; görünen adı değiştirmek bu kimlikleri rastgele değiştirmeyi gerektirmez.

## Kaynak açıklamalarının kapsamı

37 uygulama/test kaynak dosyasına dosya görevi ve fonksiyon bazında Türkçe bakım açıklamaları eklendi. C++/Rust yorum dışı token dizisi özgün kaynakla denetlendi. Üçüncü taraf upstream/Dependencies/third_party telif ve lisansları korunmuştur. Bakım üst bilgisi özgün telif sahibini değiştirmez. JSON veri dosyalarına geçersiz yorum konmadı; envanterde tarih/sahiplik kaydı tutulur.

Bu açıklama çalışması kapsamlı bir güvenlik veya tüm DAW uyumluluk sertifikası değildir. Yeni derleme mevcut yayımlanmış Release/ZIP’i veya web sitesi indirmesini otomatik değiştirmez. Eski kaynak/dağıtım materyalleri Music/_Arsiv altında tutulur; build/target gibi yeniden üretilebilen önbellekler aktif kaynak ağacına taşınmaz.

## Dosya ve fonksiyon haritası

### Source/AboutPanel.cpp

Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.

Fonksiyonlar: `Page`, `resized`, `AboutPanel::AboutPanel`, `AboutPanel::paint`, `AboutPanel::resized`.

### Source/AboutPanel.h

Geliştirici, üçüncü taraf kaynakları ve lisans/bilgilendirme görünümünü kurar.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### Source/BandGraph.h

Low/Mid/High bölme sınırlarının görsel hit-test ve sürükleme etkileşimini yönetir.

Fonksiyonlar: `BandGraph`, `~BandGraph`, `frequencyX`, `paint`, `mouseMove`, `mouseDown`, `mouseDrag`, `mouseUp`, `mouseDoubleClick`, `parameter`, `hit`, `timerCallback`.

### Source/Editor.cpp

Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.

Fonksiyonlar: `ShaperEditor::effectColour`, `ShaperEditor::ShaperEditor`, `ShaperEditor::~ShaperEditor`, `ShaperEditor::select`, `ShaperEditor::executeEdit`, `ShaperEditor::timerCallback`, `ShaperEditor::refreshPresets`, `ShaperEditor::chooseRack`, `ShaperEditor::saveRackFile`, `ShaperEditor::loadRackFile`, `ShaperEditor::refreshRackPresets`, `ShaperEditor::loadFactory`, `ShaperEditor::chooseEffect`, `ShaperEditor::saveEffectFile`, `ShaperEditor::loadEffectFile`, `ShaperEditor::showAbout`, `ShaperEditor::paint`, `ShaperEditor::resized`.

### Source/Editor.h

Ana rack editörünü, efekt sırasını, seçimlerini ve mevcut ortak kontrol yerleşimini yönetir.

Fonksiyonlar: `getTextButtonFont`.

### Source/EffectOutputPanel.h

Efektin ortak Stereo/çıkış meter/dB/Mix panelini bağlı parametrelerle eşitler.

Fonksiyonlar: `parameter`, `change`, `Dial`, `~Dial`, `paint`, `mouseDown`, `mouseDrag`, `mouseUp`, `mouseDoubleClick`, `mouseWheelMove`, `mouseEnter`, `mouseExit`, `timerCallback`, `EffectOutputPanel`, `paint`, `resized`.

### Source/EffectPalette.cpp

Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.

Fonksiyonlar: `EffectPalette::EffectPalette`, `EffectPalette::changed`, `EffectPalette::refresh`, `EffectPalette::resized`, `EffectPalette::paint`, `EffectPalette::Card::paint`, `EffectPalette::Card::mouseDown`, `EffectPalette::Card::mouseDrag`, `EffectPalette::isInterestedInDragSource`, `EffectPalette::insertionAt`, `EffectPalette::itemDragMove`, `EffectPalette::itemDragExit`, `EffectPalette::itemDropped`.

### Source/EffectPalette.h

Efekt kimliği, adı, rengi ve üretici fonksiyonunu tek kayıt sözleşmesinde tutar.

Fonksiyonlar: `rackEffectColour`, `Card`.

### Source/Embedded/JuceHeader.h

Gömülü modüller için JUCE başlık/namespace köprüsüdür; ayrı bir uygulama motoru değildir.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### Source/Embedded/PanPresets.h

Pan modülü için hazır desen ve parametre verilerini tanımlar.

Fonksiyonlar: `name`, `position`, `points`, `sync`.

### Source/Embedded/RackEditorCommands.h

Gömülü editörün seçenek komutları ve başlık yerleşimi için ortak sanal sözleşmedir.

Fonksiyonlar: `~RackEditorCommands`, `rackDockHeader`, `rackHeaderHeight`.

### Source/Embedded/RackKnob.h

Shaper standart küçük rotary çizimi ve etiket/drag alanlarını ortak tutar; görünüm standardı AGENTS.md içindedir.

Fonksiyonlar: `drawKnob`, `RackKnob`, `~RackKnob`, `paint`, `mouseDown`, `mouseDrag`, `mouseUp`, `mouseWheelMove`, `mouseDoubleClick`.

### Source/Embedded/SidechainPresets.h

Volume/sidechain yönlü hazır envelope/desen verilerinin sözleşmesidir.

Fonksiyonlar: `name`, `state`, `points`, `SidechainPresetButton`, `setPreview`, `setSelected`, `paintButton`.

### Source/Embedded/TimeQuickPresets.h

Time modülünün kısa hazır ayarlarını ve değer eşlemelerini tutar.

Fonksiyonlar: `name`, `delay`, `points`, `TimeQuickPresetButton`, `setSelected`, `paintButton`.

### Source/Embedded/VolumePresetData.h

Volume modülünün hazır desen/kontrol verisidir; veri tablosunu uygulama akışıyla karıştırmayın.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### Source/EnvelopeEditor.cpp

Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.

Fonksiyonlar: `EnvelopeEditor::EnvelopeEditor`, `EnvelopeEditor::~EnvelopeEditor`, `EnvelopeEditor::rebuild`, `EnvelopeEditor::resized`, `EnvelopeEditor::timerCallback`, `EnvelopeEditor::rackOptions`, `EnvelopeEditor::rackCommand`.

### Source/EnvelopeEditor.h

Çok noktalı envelope eğrisi, düğüm/segment düzenleme, kalıp ve taşıma etkileşimlerini yönetir.

Fonksiyonlar: `paint`, `commands`, `rackCanEdit`, `rackIsSequencing`, `rackDockHeader`, `rackHeaderHeight`.

### Source/EnvelopeEffect.cpp

Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.

Fonksiyonlar: `EnvelopeEffect::layout`, `EnvelopeEffect::EnvelopeEffect`, `EnvelopeEffect::~EnvelopeEffect`, `EnvelopeEffect::audioSettingsShown`, `EnvelopeEffect::forceCV`, `EnvelopeEffect::applyShape`, `EnvelopeEffect::isBusesLayoutSupported`, `EnvelopeEffect::prepareToPlay`, `EnvelopeEffect::releaseResources`, `EnvelopeEffect::processBlock`, `EnvelopeEffect::getProgramName`, `EnvelopeEffect::setCurrentProgram`, `EnvelopeEffect::getStateInformation`, `EnvelopeEffect::setStateInformation`, `EnvelopeEffect::createEditor`.

### Source/EnvelopeEffect.h

Bağımsız efekt motorunu tempo, tetik, sequencer ve envelope modülasyonuyla sarar.

Fonksiyonlar: `&curve()`, `getName`, `acceptsMidi`, `producesMidi`, `getTailLengthSeconds`, `hasEditor`, `getNumPrograms`, `getCurrentProgram`, `changeProgramName`.

### Source/EnvelopePresets.h

Envelope biçimlerini ve hazır desen verilerini saklar; sıra/alanlar kaydedilen durumla uyumludur.

Fonksiyonlar: `name`, `set`, `reset`, `value`, `curve`.

### Source/FactoryPresets.cpp

Rack fabrika presetlerinin efekt dizilişi ve kaydedilen parametre durumlarını tanımlar.

Fonksiyonlar: `plain`, `curve`, `shape`, `ShaperProcessor::factoryPresetName`, `ShaperProcessor::loadFactoryPreset`.

### Source/ForwardParameter.h

İç efekt parametresini dış host parametresine yönlendirir; host gesture ve değer ölçeklerini korur.

Fonksiyonlar: `ForwardParameter`, `detach`, `~ForwardParameter`, `getValue`, `setValue`, `getDefaultValue`, `getText`, `getValueForText`, `getNumSteps`, `isDiscrete`, `isBoolean`, `&getNormalisableRange() const override`, `parameterValueChanged`, `parameterGestureChanged`.

### Source/IconButton.h

Rack düğmelerinin görsel/etkileşim durumunu ve ortak ikon çizimini tanımlar.

Fonksiyonlar: `CompactIconButton`, `paintButton`.

### Source/MultibandProcessor.cpp

Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.

Fonksiyonlar: `setPlain`, `rebuild`, `panelWidth`, `commands`, `timerCallback`, `BandEditor`, `~BandEditor`, `paint`, `resized`, `rackOptions`, `rackCommand`, `rackCanEdit`, `rackIsSequencing`, `MultibandProcessor::MultibandProcessor`, `MultibandProcessor::outputParameter`, `MultibandProcessor::~MultibandProcessor`, `MultibandProcessor::getName`, `MultibandProcessor::setCurrentProgram`, `MultibandProcessor::isBusesLayoutSupported`, `MultibandProcessor::setMultiband`, `MultibandProcessor::prepareToPlay`, `MultibandProcessor::releaseResources`, `MultibandProcessor::processBlock`, `MultibandProcessor::createEditor`, `MultibandProcessor::getStateInformation`, `MultibandProcessor::setStateInformation`, `MultibandProcessor::resetBands`, `MultibandProcessor::readSpectrum`.

### Source/MultibandProcessor.h

Üç bağımsız efekt motoru, bant ayırma/birleştirme, bypass ve bant durumu saklamasını yönetir.

Fonksiyonlar: `hasEditor`, `acceptsMidi`, `producesMidi`, `getTailLengthSeconds`, `getNumPrograms`, `getCurrentProgram`, `getProgramName`, `changeProgramName`, `&band(int b)`, `&current()`, `hasLowBand`, `hasHighBand`, `isMultiband`, `editorBand`.

### Source/Pitch/PitchProcessor.cpp

Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.

Fonksiyonlar: `prepare`, `Processor::layout`, `Processor::Processor`, `Processor::~Processor`, `Processor::isBusesLayoutSupported`, `Processor::prepareToPlay`, `Processor::releaseResources`, `Processor::reset`, `Processor::processBlock`, `Processor::processModulated`, `Processor::getProgramName`, `Processor::setCurrentProgram`, `Processor::getStateInformation`, `Processor::setStateInformation`, `Processor::createEditor`, `createEmbeddedProcessor`, `createAboutPage`.

### Source/Pitch/PitchProcessor.h

Pitch/formant motorunu Signalsmith işleme durumuyla bağlar; gecikme, reset ve kanal durumunu taşır.

Fonksiyonlar: `getName`, `acceptsMidi`, `producesMidi`, `getTailLengthSeconds`, `hasEditor`, `getNumPrograms`, `getCurrentProgram`, `changeProgramName`.

### Source/Processor.cpp

Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.

Fonksiyonlar: `ShaperProcessor::effectName`, `ShaperProcessor::layout`, `ShaperProcessor::ShaperProcessor`, `ShaperProcessor::palette`, `ShaperProcessor::addEffect`, `ShaperProcessor::removeEffect`, `ShaperProcessor::moveEffect`, `ShaperProcessor::signalOrder`, `ShaperProcessor::applyPaletteOrder`, `ShaperProcessor::~ShaperProcessor`, `ShaperProcessor::isBusesLayoutSupported`, `ShaperProcessor::prepareToPlay`, `ShaperProcessor::releaseResources`, `ShaperProcessor::handleAsyncUpdate`, `ShaperProcessor::processBlock`, `ShaperProcessor::render`, `ShaperProcessor::getStateInformation`, `ShaperProcessor::setStateInformation`, `ShaperProcessor::createEditor`, `JUCE_CALLTYPE createPluginFilter`.

### Source/Processor.h

Rack efekt zinciri, MIDI/transport, preset, ses callback ve kaydedilen durum akışını yönetir.

Fonksiyonlar: `hasEditor`, `getName`, `acceptsMidi`, `producesMidi`, `isMidiEffect`, `getTailLengthSeconds`, `getNumPrograms`, `getCurrentProgram`, `setCurrentProgram`, `getProgramName`, `changeProgramName`, `&module(int effect)`, `enableId`, `presentId`, `getPosition`, `prepare`, `sample`, `advance`.

### Source/Saturation/PluginProcessor.cpp

Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.

Fonksiyonlar: `Processor::layout`, `Processor::Processor`, `Processor::settings`, `Processor::prepareToPlay`, `Processor::processBlock`, `Processor::isBusesLayoutSupported`, `Processor::createEditor`, `Processor::getProgramName`, `Processor::setCurrentProgram`, `Processor::getStateInformation`, `Processor::setStateInformation`, `createEmbeddedProcessor`, `createAboutPage`.

### Source/Saturation/PluginProcessor.h

Saturation parametrelerini, presetleri ve host callback’lerini kendi DSP motoruna bağlar.

Fonksiyonlar: `releaseResources`, `reset`, `getName`, `acceptsMidi`, `producesMidi`, `getTailLengthSeconds`, `hasEditor`, `getNumPrograms`, `getCurrentProgram`, `changeProgramName`.

### Source/Saturation/SaturationDSP.cpp

Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.

Fonksiyonlar: `coefficient`, `follow`, `SaturationDSP::ADAA::logCosh`, `SaturationDSP::ADAA::shape`, `SaturationDSP::ADAA::process`, `SaturationDSP::Magnet::langevin`, `SaturationDSP::Magnet::langevinDerivative`, `SaturationDSP::Magnet::slope`, `SaturationDSP::Magnet::process`, `SaturationDSP::Model::process`, `SaturationDSP::compensationDb`, `SaturationDSP::preview`, `SaturationDSP::prepare`, `SaturationDSP::reset`, `SaturationDSP::process`, `SaturationDSP::processModulated`, `SaturationDSP::render`.

### Source/Saturation/SaturationDSP.h

Doygunluk karakterleri, oversampling, ADAA, dinamik hafıza, filtre ve hizalı dry/wet zincirini uygular.

Fonksiyonlar: `getLatency`.

### Tests/AudioTests.cpp

AudioTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `getPosition`, `set`, `preview`, `snapshot`, `byId`, `finite`, `main`.

### Tests/EnvelopeTests.cpp

EnvelopeTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `set`, `find`, `getPosition`, `main`.

### Tests/PitchTests.cpp

PitchTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `set`, `finite`, `main`.

### Tests/SaturationDSPTests.cpp

SaturationDSPTests regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `snapshot`, `render`, `rms`, `aliasDb`, `harmonicDb`, `wave`, `main`.


## C# bilen geliştirici için C++ kodunu okuma

`.h` sınıf/sözleşmeyi, `.cpp` uygulamayı taşır; header içindeki inline DSP fonksiyonları da gerçek işleme kodudur. `unique_ptr` tek sahip, referans/ham pointer çoğunlukla ödünç erişimdir. Sahipliği bilmeden delete eklemeyin. Kurucu hazırlığı, yıkıcı kaynak bırakmayı yapar; JUCE host’un sahip olduğu Processor/Editör ve attachment ömürleri farklıdır. Audio callback ile message thread aynı anda çalışabilir. Normal Debug build PDB üretir; Release her değişkeni debugger’da göstermeyebilir. Yeni hatayı küçük sentetik testle tekrar üretip Call Stack ve gerçek fiziksel birimlerden ilerleyin.
