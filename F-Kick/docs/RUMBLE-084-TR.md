# F-Kick 0.8.4 — Rumble kullanım rehberi

Fazıl Irak · 03.11.2026

## Üç ayrı yaklaşım

FX alt başlığındaki seçim Classic, Groove veya Hybrid motorunu seçer. Pencere 912 × 432 olarak korunur. Routes ve Bass, aynı alt şeridi kullanır; Sound/FX/SC ana sayfaları korunmuştur. Limit yalnız rumble dönüşünün tepesini sınırlar, MIDI root ise Hybrid bass'ın nota takibini açar.

**Classic:** Eski dört comb motoru → iki kademeli HP → Drive → iki kademeli LP → Amount/Gain. Echo, Kick veya işlenmiş Rumble kaynağını kullanır. Room ayrı paralel dönüş üretir. Routes üzerinde Legacy sum ve Limit kapalı, önceki sürümün ses yoludur. 18 gerçek eski/yeni DSP karşılaştırmasında örnek farkı sıfırdır. Split returns isteğe bağlı yeni davranıştır.

**Groove:** Classic'in comb karakterinin üstüne 16 hücreli özgün dönüş seviye eğrileri gelir. Offbeat, Rolling, Broken ve Syncopated birer 4/4 bar boyunca çalışır. Groove derinliği rumble'a tam, echo'ya %65 oranında katılır; Room etkilenmez. Echo Sync/Division ile zamanlamayı ayrıca ayarlayın. Bu yalnız bir reverb preset değişimi değildir: dönüşlere tempo tabanlı genlik şekillendirmesi eklenir. Groove sıfırken ilave ritmik kesme olmaz.

**Hybrid:** Groove yanında ayrı bir mono bass sentez motoru çalışır. Kick kuyruğundan veya reverb çıkışından türetilmez. Sinüs temel ve sınırlı 2–4. harmonikler, kısa attack, üstel decay ve low-pass kullanır. Rolling, Offbeat, Driver ve Octaves sabit nota kalıplarıdır; bir sequencer editörü yoktur. Aynı bass sinyali L/R'ye eklenir. Root MIDI 29 = F1 (~43.65 Hz), 41 = F2. Tune yarım ses birimindedir. MIDI root açıkken MIDI 60 referansına göre transpoze edilir.

## Saat ve tetikleme

DAW çalışırken geçerli PPQ konumu bar fazını belirler. Dahili/fallback saat ilk MIDI tetiklemesinde sıfırlanır ve sonra sürer; her kick'te başa dönmez. Tempo mevcut 40–300 BPM aralığındadır. Hybrid yeni adımları son MIDI tetiklemesinden sonraki dört beat boyunca başlatır; son notanın decay kuyruğu devam edebilir. Yeni kick bu süreyi uzatır. Bu nedenle tek nota sonrası dört beat bass duymak normaldir. Panic bass'ı ve sonraki pattern notalarını da durdurur. MIDI note-off'un bass pattern'ini anında kesmesi beklenmemelidir.

## Routes: dönüşleri ayrı yönetme

R = Rumble, E = Echo, Rm = Room, B = Hybrid bass. Duck sütunları mevcut Duck/Recover davranışına katılım, SC sütunları mevcut dört şekilli SC Mix/Shift davranışına katılımdır. %0 o işlemi ilgili dönüşten çıkarır; %100 tam uygular. Kuru kick/sample bu katılım ayarlarıyla değişmez. Katılım değişimleri 2 ms yumuşatılır. Echo ve Room Gain yalnız Split yolundaki bağımsız dB trim'leridir; Rumble Gain mevcut FX sayfasındadır, bass Level Bass sayfasındadır. Groove/Hybrid Split'i otomatik kullanır. Classic Legacy'de yeni katılım/trim alanları pasiftir.

Ceiling, Limit açıkken rumble dönüşünün dBFS örnek tepe sınırıdır. Stereo-linked algılama iki kanalı birlikte kısar; attack anlık, bırakma yaklaşık 80 ms'dir. Ek gecikme/lookahead yoktur; true-peak veya mastering limiter değildir. Son toplam kick+echo+room+bass seviyesini sınırlamaz. Bypass eski sesi korur. Toplam çıkış ve DAW metresini ayrıca izleyin.

FX HP/LP rumble+echo+room toplamını filtreler; bağımsız bass bu filtreleri atlar. Genel HP ve 300 Hz Bass EQ kuru ses, dönüşler ve bass toplamını etkiler. Bass Cutoff yalnız bass'ı şekillendirir. Hybrid açma/kapatma yaklaşık 10 ms geçiş kullanır; aktif ses üzerinde aşırı Root/Tune/Decay otomasyonu için ayrıca dinleme yapın.

## Başlangıç reçetesi

1. Options (…) → Rumble recipes menüsünden 12 ek özgün tariften birini seçin. İlk 128 factory preset değiştirilmedi. Tarifler ayrıca validation/Rumble-Recipes içinde .fkick ve 48 kHz WAV olarak bulunur.
2. Classic ile eski karakteri dinleyin; Groove'da Offbeat/Syncopated ve Groove derinliğini karşılaştırın.
3. Hybrid'de kick Length'i kısa tutun, bass Root'u parçanın tonalitesine göre seçin. Bass Level ve Rumble Amount ile iki alt frekans katmanının birbirini örtmesini azaltın. R/B Duck kick başlangıcına yer açar; Recover uzunluğu bir sonraki nota gelmeden dönüşün toparlanmasını belirler.
4. Dönüşleri karşılaştırmak için Rumble/Echo/Room Amount ve bass Level'i sırayla kapatın. Önce tek tek, sonra birlikte ve mono dinleyin. Rumble limiter'ını yalnız ihtiyaç varsa açın; toplam çıkıştaki kırpılmayı çözmek için Output'u düşürün.

## DAW'da kaydetme ve yeniden düzenleme

Bir–iki bar düzenli kick notası çalın; Groove/Hybrid'i ayrı audio kanala kaydedin veya resample edin. Kayıt kuyruğunu da alın. Grid üzerinde beğendiğiniz aralıkları kesip yeni audio kliplere yerleştirin; kısa fade'ler kullanın. Kaydı dry kick ile katmanlarken başlangıç hizasını ve mono faz ilişkisini dinleyin. F-Kick içinde audio dilimleyici ya da kullanıcı pattern editörü eklenmedi.

## Uyumluluk ve doğrulamanın sınırı

Eski 131 parametrenin ID/sırası ve FKICK_PRESET v1 korunur. Yeni 24 parametre sona eklenir; eski kayıtlarda Classic/Legacy/Limit off varsayılanı kullanılır. 12 yeni tarif, 44.1/48/96 kHz sonlu çıkış, preset/state, blok boyutu, limiter, bağımsız bass ve GUI testleri geçti. Otomatik render testleri müzikal kalite onayı değildir. Kullanıcının DAW'ında kulakla karşılaştırma ve tempo/otomasyon denemesi bekliyor. Bu teslim Windows VST3 ve bağımsız EXE'dir; macOS/Linux derlemesi yapılmadı.
