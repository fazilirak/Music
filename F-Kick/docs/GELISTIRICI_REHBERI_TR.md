# F-Kick geliştirici rehberi

Fazıl Irak · 27.10.2026 · 0.7.0

## Dosya haritası

| Dosya | Sorumluluk |
|---|---|
| Source/Engine.h/.cpp | Sabit 16 voice, sample okuma, gövde sentezi, eğri değerlendirme, filtre/DC/drive/çıkış |
| Source/PluginProcessor.h/.cpp | APVTS, örnek doğruluğunda MIDI, worker I/O, sample sahipliği, preset/state, çevrimdışı WAV |
| Source/PluginEditor.h/.cpp | Ortak tema, küçük kontroller, sample/curve görünümü, dosya seçici, logo/About |
| Tests/AudioTests.cpp | Gerçek DSP/host/state/GUI yolları, ses ve PNG kanıtları |
| scripts/build-windows.cmd | VS keşfi, CMake/Ninja, build ve CTest |
| scripts/package.ps1 | Yerel sürümlü ikili/kaynak paketleri ve SHA-256 |
| Resources/ ve licenses/ | Değiştirilmemiş font ve lisans metinleri |

## Ses yolu

MIDI → voice seçimi → sample (resampling/trim/loop/reverse) + sinüs Body/harmonikler →
ses şiddeti zarfı/velocity → ortak low-pass → Soft/Clip/Fold Drive → paralel rumble/echo → bağımsız HP → DC blocker → Output → peak clip.
Sample ve Body kazancı, cutoff, drive ve output 8 ms rampalanır. Diğer ayarlar blok başında
okunur; phase yeni notada alınır. Çizim eğrileri sabit kapasiteli dizilere kopyalanır.

Callback örnekleri temizler, pending sample'ı güvenle devralır, MIDI olayları arasındaki aralıkları
render eder. MIDI için ek gecikme bildirilmez. Polifonik ses çalma en eski voice'u devralır;
64 örnek kısa geçiş son ses değerinden yeni sese geçer. Sustain pedalı kanal bazlıdır.
Kick sonunda 3 ms güvenlik fade'i, WAV sonunda 5 ms sınır fade'i uygulanır.

Sample verisi immutable'dır. Worker/host yolu hazırlanmış sample ham işaretçisini atomic pending
alana bırakır. Callback önceki sample'ı AbstractFifo retirement kuyruğuna yollar; sonraki load/clear
ve destructor bunları audio dışında siler. Kuyruk dolarsa callback sahiplik değiştirmeyi erteler.
Aktif sample'ı editor okumaz; UI için ayrı 512 peak özeti ve meta veri kopyası vardır. sampleLock
yalnız worker/state/UI arasında kullanılır, processBlock ve Engine hiçbir kilide girmez.
Worker ThreadPool işlemciye aittir ve destructor tüm işleri bitirir; asenkron dosya seçiciler
Component::SafePointer ile editor kapanışını korur.

## Parametre ve state sözleşmesi

APVTS root `FKICK_PARAMETERS`; dış binary-XML root `FKICK_PRESET`, version 1.
24 ana parametreye iki bağımsız, en fazla sekiz noktalı eğri eklenir. Eğri count ve X/Y/Curve
alanları host parametresidir. Mevcut ID'leri değiştirmeyin. Yeni parametre için layout → cache →
settings → DSP → UI → round-trip testi sırasını izleyin. Fiziksel Hz/ms/dB ile normalize 0..1'i
karıştırmayın. Attack/Release DSP içinde saniyeye, gain'ler lineer genliğe çevrilir.

Sample WAV state içinde base64 tutulur; orijinal kaynak 24-bit PCM'e dönüştürülür. İçe aktarma
en fazla 30 saniye, 1/2 kanal, 8–192 kHz ile sınırlandırılır. XML ve WAV boyut sınırları vardır.
Başarısız import/restore önceki state'i korur. Source dosyası adı bilgi amaçlıdır; restore bu yoldan
dosya açmaz. Fabrika patch'leri sample'ı silmez; Clear sample ayrı komuttur.

## Derleme

Ortak JUCE revision: `10a589619b452c261b2940767eb253171eb5a823`. Üçüncü taraf kaynakları değiştirmeyin.
Windows: `scripts/build-windows.cmd`. Elle: CMake Ninja Release, JUCE_PATH; ardından build ve CTest.
Test executable'ına çıktı klasörü parametresi verilebilir. PNG'ler gerçek JUCE bileşenlerinin
snapshot'ıdır, maket değildir. Dağıtım dizinine ancak build/test ve görsel kontrol sonrası geçin.

## 0.2.0 yerleşim ve kontrol sözleşmesi

Pencere 760 × 360; önceki 880 × 570 alanından yaklaşık %45 daha az yer kaplar. Görsel
referans, Ableton Live 12 kılavuzundaki Simpler Classic/One-Shot/Controls ekranlarıdır:
https://www.ableton.com/en/live-manual/12/live-instrument-reference/#simpler
Referans görselleri inceleme içindir; projeye üçüncü taraf UI varlığı kopyalanmadı.

`FKickLogo` normal Component'tir; Button değildir ve çerçeve çizmez. Mouse-up veya Return/Space
About'u açar. `preset-previous` ve `preset-next` ComboBox'ın iki yanında gerçek setCurrentProgram
çağrısı yapar, ilk/son arasında döner. Eski oklar Undo/Redo idi; bu işler artık yalnız `...` menüsündedir.
`MidiKeyboardComponent` ve rotary bankaları kaldırıldı; host/MIDI motoru ve küçük audition üçgeni durur.

`FKickValue` ParameterAttachment ile aynı parametrelere bağlanan metin kontrolüdür. Drag/wheel
normalleştirilmiş aralıkta, Shift ince ayarda çalışır. Double-click Label editor'ünü açar; Alt-click
resetler. Start/End hem drag hem yazıyla girişte birbirini geçemez. NumberLabel wheel'i parent'a
ikinci kez iletmez. Attachment ve gesture ömrünü değiştirmeden koruyun; callback'e UI kodu eklemeyin.
Sample değerleri sample panelinin alt satırındadır. Body/Envelope/Filter ayrı dar gruplardır.
Host parametre kimlikleri, FKICK_PRESET version 1 ve DSP değişmemiştir; eski presetler okunur.

## 0.2.1 dosya bırakma ve trim görünümü

`FKickSampleDropArea` gerçek `FileDragAndDropTarget` ve sample kontrollerinin ebeveynidir.
JUCE ComponentPeer hedef araması waveform veya bir değerin Label'ından yukarı yürüdüğünde
doğrudan bu paneli bulur. Editor yalnız `.fkick` preset bırakmalarını kabul eder; sample
yükleme panelin kendi hedefidir. Yeni layout eklerken sample kontrollerini yeniden editörün
kardeş bileşenlerine taşımayın. resized içindeki editör→panel koordinat dönüşümünü koruyun.
Hover yalnız paintOverChildren'da çizilir; bırakma/çıkışta temizlenir. Tek dosya, format ve
dosyanın varlığı kontrol edilir. Kabul edilen dosya mevcut worker tabanlı loadSampleAsync
yoluna gönderilir; ses callback'inde dosya açılmaz. DSP/state/host kimlikleri değişmedi.

Sample tabında dalga çizildikten sonra Start öncesi ve End sonrası plot bölgesine %42 siyah
saydam katman çizilir. Seçili aralık değişmez; flag ve açıklamalar karartmadan sonra çizilir.
Pitch/Amp zaman eksenleri sample trim ekseni olmadığı için onlara maske uygulanmaz.

Test, native ComponentPeer::DragInfo hedef bulma ve asenkron drop teslimini hem waveform
hem Label üzerinden sınar; Unicode/boşluklu dosya yolu ve geçersiz drop da kontrol edilir.
Bu native arayüz testi gerçek bir Windows Explorer/Ableton oturumundaki elle sürükleme testinin
aynısı olarak raporlanmamalıdır. Kabul edilen dosyanın yüklenmesi ve arayüz yenilenmesi de sınanır.

## Sonraki geliştirmeler

Kullanıcı tarafından istenen bekleyen işler: [YAPILACAKLAR_TR.md](YAPILACAKLAR_TR.md).

Öncelikler: daha yumuşak loop crossfade, antialiasing/oversampling, sample audition/export nota
seçimi, wavetable/çoklu sample yerine sade katman yapısının korunması. Warp, analiz ve transient
slicing ayrı DSP tasarımı ve test gerektirir; mevcut sürümde çalışıyor gibi gösterilmemelidir.
Gerçek Ableton tarama/yeniden açma ve ses cihazında dinleme ayrı host kabul testidir.


## 0.3.0 techno motoru ve sözleşmeler

`Source/TechnoFX.h/.cpp` sabit MotionFrame ve prepare sırasında ayrılan stereo delay/comb tamponlarını içerir. Dört comb stereo dağılmış rumble üretir; HP → Drive → LP ayrı wet yoludur. Echo kesirli okuma, LP feedback ve tanh geri besleme sınırı kullanır. 2.4 saniyelik ring en yavaş desteklenen sync süresini kapsar. Duck nota anında wet kollarını indirir, Recover ile üstel döner. Panic efektleri 5 ms içinde susturur; sonraki nota tamponları tahsis yapmadan temizler. Ana HP Off iken eski nötr yol bit düzeyinde korunur.

Motion her örnekte hesaplanır: ortak LFO + tek atımlı AD. Her hedef iki kaynağın signed toplamını alır. Per-voice modülasyon değildir. Retrigger her note-on'da; serbest LFO yalnız hız senkronludur, transport fazına bağlanmaz. Host BPM alınamazsa manuel Tempo kullanılır. Modülasyon sonrası değerler fiziksel güvenlik sınırına kırpılır. Echo feedback tabanı en çok 0.8; modülasyonla 0.85 sınırı kullanılır. Üç drive karakterinde oversampling yoktur.

Önceki 74 host parametresinin ID ve sırası korunur; 27 FX/Motion + 20 routing parametresi sona eklenir. Routing ID biçimi `lfo_<target>` / `ad_<target>`, hedefler TechnoFX içindeki modIDs dizisidir. FKICK_PRESET version 1 devam eder. Restore, eksik bilinen parametreleri güncel instance değerinden değil fiziksel varsayılandan tamamlar. APVTS XML çocuk sırası host parametre sırası değildir; eski preset testi yeni ID'leri açıkça çıkarır.

UI Sound/FX/Mod sayfasını görünürlükle değiştirir. Sample kontrolleri native drop panelinin altındadır. Sağ tık menüsü ve Mod Routing aynı APVTS parametrelerine yazar; hedef seçimi önceki bağlantıları korur. 0.4.0 itibarıyla HP de metindir; rotary istisnası kaldırıldı. Motion grafiği ve renkler GUI timer üzerinden atomic metrelere bakar.

WAV export ve host tail bildirimi decay/echo feedback ile modülasyon derinliğini dikkate alır; kuyruk tahmini -60 dB için en çok 120 saniye ile sınırlıdır. Bu algısal sessizlik garantisi değildir. Son WAV fade'i korunur. Tests/TechnoTests.cpp filtre, duck, echo zamanlama, LFO/AD, ses farkı, blok bağımsızlığı, ekstrem drive/rate, state migration, sayfalar ve eşzamanlı routing'i sınar; GUI PNG ve dört WAV üretir.

## 0.4.0 banka, menü ve native aktarım

Ana banka `Resources/FactoryBank.json`, üretici `scripts/make_factory_bank.py`, derlenmiş veri `Source/FactoryBank.h` içindedir. Generator deterministiktir; dış sample veya rastgele seed kullanmaz. JSON ve header birlikte güncellenmelidir. İşlemci bankayı GUI/host program seçiminde uygular; audio callback dosya okumaz. Sekiz klasik ve üç deneysel kontur, pitch/amplitude noktalarını ayrı şekillendirir. Banka 64/48/16 olarak sabittir; 128 portable .fkick test tarafından üretilir. Yeni factory sesleri sampleGain'i Off yapar; GUI'den yeni sample yükleme onu -6 dB açar.

Eski program 8–11 → 64–67 taşınır; eski 7 Sampler Init sesi kendi state parametrelerini korur, program etiketi 0'a eşlenir. Sesin parametreleri patch seçerek yeniden oluşturulmaz. Yeni state ek `bankRevision=2` taşır; FKICK_PRESET version 1 ve parametre ID'leri korunur. Önceki Long 808 fabrika tarifi artık one-shot Kick modundadır; eski state'in modu değiştirilmez.

lfoShape fiziksel indeksleri 0 Sine, 1 Triangle, 2 Saw Up, 3 Square, 4 Random, 5 Saw Down. Görsel sıra Saw Up/Down'ı yan yana getirir. Standart ComboBoxAttachment seçilen item indeksini kullandığından burada ParameterAttachment ile ID–değer eşlemesi yapılır. Eski presetlerin fiziksel değerleri korunur; eski DAW normalized choice otomasyon eğrileri yeni seçenek sayısı nedeniyle ayrıca gözden geçirilmelidir.

NativeFileDrop yalnız editörün JUCE peer HWND'sine OLE kaydı yapar. CF_HDROP verisi Windows DragQueryFileW ile okunur ve STGMEDIUM serbest bırakılır. Ana host HWND'si değiştirilmez. Timer peer hazır olunca kayıt yapar; destructor kayıt/COM oturumunu bırakır. Callback sample/preset yüklemeyi mevcut GUI/worker yoluna aktarır. Gerçek Explorer hareketi doğrulanmadığından bunun bütün hostlarda kesin çözüm olduğu iddia edilmez. CTest gerçek OLE IDataObject ve Unicode dosya aktarımını, native Drop callback'ini ve kayıt başarısını kontrol eder.

BankTests her preset için 44.1/96/192 kHz kararlılık ve 48 kHz üç vuruşlu headroom/DC/kuyruk ölçümü yapar; state geri dönüşü ve 128 menü seçimi/iki yönde tam dolaşımı test eder. CSV ölçümleri ve üç ses seçkisi paketlenir. Render hash'lerinin farklı olması tek başına algısal kalite veya farklılık garantisi değildir; isimler ve tarifler ayrıca katalogda açıklanır.

Çıkış kalibrasyonu `Resources/FactoryLevels.json` içinde sabittir; üretici bunu son aşamada tariflere uygular. Değerler 48 kHz/135 BPM/velocity 127 üç vuruş ölçümünden alınır. Yeni sesler yaklaşık −6 dBFS peak hedefler; ilk yedi temel presetin seviyesi korunur.

## 0.5.0 impact ve wet kazancı

Host sırası 0–120 korunur; `punch` indeks 121, `rumbleGain` indeks 122 olarak sona eklendi. Yeni varsayılanlar .65 ve 0 dB; eski state'te bulunmayan iki alan açıkça 0'a taşınır. Anahtar/state sürümü aynı kalır. Factory seçimi yeni konturları ve yoğunluğu yükler, eski custom state kendi eğrilerini ve eski oscillator/wet kazancını korur.

Punch 8 ms rampalanır. Kick Body'de faz kilitli ikinci harmonik ilk 45 ms üstel azalır; ardından normalize tanh ile gövde yoğunluğu artırılır. Sample/sampler yolu etkilenmez. Bu aşama oversampling içermez. Rumble Gain 20 ms rampalanan bağımsız post-tone wet kazancıdır; Echo'nun rumble kaynak girişi bu dönüş trim'inden önce kalır. Geri besleme kazancı değiştirilmez.

Fabrika konturlarında ilk amplitude tepesi ve pitch yerleşmesi milisaniye hedeflerinden normalize noktalarına çevrilir. Kullanıcının düzenlediği eğri sürekli zorlanmaz; bu dönüşüm yalnız factory seçerken yapılır. 8–10 deneysel kontur ayrı tutulur.

ImpactTests iki 0.4 state fixture'ı ile nötr migration'ı ve +12 dB izole wet dönüşünü kontrol eder. Kuru A/B dosyaları aynı 0.5 peak'e normalize edilir; 20–150 ms RMS karşılaştırması salt output artışından ayrılır. Rumble ölçümünde dry render toplam render'dan çıkarılır. Otomatik metrikler fiziksel dinlemenin yerine geçmez.

0.5.0 son kalibrasyon: `Resources/ImpactLevels.json` master çıkışları, `Resources/RumbleLevels.json` wet dönüş trim'lerini sabitler. Generator önce eski 0.4 tarifini kurar, sonra bu güncel değerleri uygular. Gain aralığı −24…+24 dB'dir. 069 RMS ölçümünde +10.70 dB gövde farkı vardır (20–150 ms, equal peak); wet-between/dry-hit oranı yaklaşık .45. Rumble dengeleme ölçümü 120 BPM dört vuruş, banka genel peak testleri 135 BPM üç vuruştur.

C3/MIDI60 için tracking transpozisyonu sıfırdır; kullanıcının somut 069 şikâyeti envelope/wet dengesiydi. Varsayılan tracking ayrıca kapatıldı; MIDI36/60 kick render'ı eşitlik testi vardır. Sampler yolu `mode==1` koşuluyla nota takibini korur. Legacy Deep House fixture export'u önceki 0.4 WAV ile byte-byte aynıdır; SHA-256 610d992b8a121c41038ecede9dcbb9a8c4958333c9ee474249e7de66207d086e. Yeni factory Deep House sesi bilinçli olarak değişmiştir.


## 0.6.0 FX shaper ve room

Source/FXShaper.h dört özgün kapalı form kazanç eğrisi içerir. F-Sidechain'in phase/mix davranışı incelendi; F-Kick'te bütün eğri döngü içinde kaydırılır. Son %2 yumuşak kapanır; gain'de ayrıca 2 ms anti-click vardır. Mix/Shift ve filtreler 20 ms rampalanır. Rate sabit 1/4 notadır.

Önceki 123 ID/indeks korunur; sona scShape, scShift, scMix, room, fxHP, fxLP (123–128) eklendi. Mix=0, Room=false, HP=0, LP=20000 eski sesi tam bypass ile korur. FKICK_PRESET v1/bankRevision2 sürer. 0.5 golden WAV byte-byte eşittir.

İşlemci PPQ/BPM/playing bilgisini blok başında alır; pending sample reset'inden sonra Engine::syncFX çağrılır. TechnoFX fazı PPQ kesrine bağlayıp örnekler boyunca ilerletir. Host çalarken MIDI fazı sıfırlamaz; transport yokken note-on tetikler. Negatif PPQ ve seek modulo ile karşılanır.

Dönüş yolu: rumble + echo + paralel room → FX HP/LP → SC → Duck/Recover + panic → dry ile toplam → mevcut global HP/DC/output. Yeni işlemler dry dalını değiştirmez. Filtreler iki kaskat TPT tek-kutuplu aşamayla 12 dB/oct'tur.

Room, JUCE Reverb kullanır: roomSize .25, damping .65, width .85, wet=1/dry=0, dönüş .28. Tamponlar prepare'da ayrılır. İlk JUCE coefficient rampası prepare'da sıfırlarla tamamlanır; aksi halde ilk 10 ms kuru kopya sızması ve fresh/reused render farkı oluşuyordu. Regresyon testleri bunu kapsar. Açılışta clear, kapanışta fade; panic 5 ms. Room kuyruk bildirimi 2.5 saniyedir.

UI: page0 Sound, page1 FX, page2 SC, page3 Advanced modulation. SC grafiği ve FX footer native drop panelinin çocuklarıdır. FX footer sample kontrollerinin yerini yalnız FX görünümünde alır. Sample düzenleme Sound'da korunur.

Tests/FXRackTests.cpp: dört eğri, signed shift, Mix, dry koruma, room ilk reflection öncesi dry kopya testi, tail, HP/LP attenuation, PPQ/seek/MIDI fallback, 127/512 blok eşitliği, state, 0.5 golden WAV, gerçek mouse-event graph drag, UI/PNG/WAV kanıtı.


## 0.7.0 Room Mix ve sağ sütun

room ID/indeks126 korunarak bool yerine float(0..1) oldu. Eski false/true, Mix 0%/100% uçlarıdır;
DSP .28 * room ile dönüşü rampalar. Parametre sayısı 129, FKICK_PRESET sürümü aynıdır. .37 state roundtrip ve
0.6 Room On + SC-1 WAV fixture'ı yeni %100 sesiyle byte-byte eşitliği sınanır. Pozitif ara değerlerde reverb reset olmaz.

Grafik 76,72 / 570×178. x660 sağ sütun: Room Mix y77, FX HP y121, FX LP y165, Tempo y209.
FKickValue::setInlineCaption ilk beş sample alanında kullanılır; bu alanlar grafiğin kardeşleri olduğu için
toFront(false) ile paint ve hit-test sıralamasında üstte tutulur. Plot yüksekliği de artar; trim piksel testi plotBounds kullanır.
SC grafiği aynı boyutu kullanır. Room/HP/LP bütün ana sayfalarda görünür.

LFO/AD UI, seçenekler menüsü ve sayısal alanların sağ-tık mapping arayüzü kaldırıldı. DSP/parametreler/eski mapping'ler
korunur. Pitch/Amp nokta düzenleme kick'in temel aracı olarak sürer. Eski mod/saw UI PNG'leri güncel validation'dan çıkarılır.
FXRackTests sürekli Room tipi, %50 wet RMS, .37 state, sağ sütun, plot yüksekliği ve inline değerlerin görünür UI ağacında
mouse hit-test ile grafiğin üstünde kalmasını kontrol eder.


## 0.8.0 — toplam çıkış Bass EQ

F-EQ/src/dsp.rs ve params.rs referans alındı; F-EQ dosyaları değiştirilmedi. BassEQ.h,
standart biquad low-shelf denklemini sabit 300 Hz, ±12 dB ve Q 0.1–2.0 için uygular.
F-EQ ile aynı Q tanımı: t = sin(w)/Q * sqrt(A). Varsayılan Q .707, gain 0 dB'dir.

bassLift ve bassQ host parametreleri index 129/130 olarak sona eklenir; toplam 131.
Eski 129 parametrenin ID/sırası değişmez. FKICK_PRESET v1 eksik alanları varsayılanla tamamlar.
Banka yükleme bu EQ'yu sıfırlar; önceki factory sesleri değişmez.

TechnoFX sonundaki toplam dry+FX ve seri ana HP çıkışına uygulanır; Engine output/peak
korumasından öncedir. HP/LP return routing değişmez. SC kuru kick'i etkilemez.
Gain/Q 20 ms rampalıdır. Katsayılar yalnız değiştiklerinde hesaplanır; stereo TDF2 hafıza
sabit boyutludur. Render/begin içinde allocation/lock/I/O yoktur. 0 dB bit-exact bypass.

Sağ sütun: Room Mix (660,77,81,38), HP (660,121,38,38), LP (705,121,38,38),
Bass Lift (660,165,51,38), Q (716,165,27,38), Tempo (660,209,81,38).
Bass Lift/Q sample drop parent zinciri altındadır. Grafik 570×178 ve dış pencere 760×360 kalır.

FXRackTests shelf gain/merkez/tiz tepkisini 44.1/48/96/192 kHz, gain ±12 ve Q uçlarında ölçer.
Ayrıca Q etkisi, stereo ayrılığı, 0 dB bypass, otomasyon, toplam routing, state, blok eşitliği,
eski WAV uyumluluğu ve yeni sayısal UI alanları doğrulanır.

## Kaynak yayını 0.8.1
Özgün uygulama kodu ve DSP: Fazıl Irak. Standart dosya görev başlıkları ve ana ses yollarına bakım yorumları eklendi. Üçüncü taraf sahiplik/lisansları AUTHORS.md ve mevcut bildirimlerde korunur. Bu bakım sürümü parametre kimliklerini veya ses algoritmasını değiştirmez.

