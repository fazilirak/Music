# Model F 0.3.0 — Bağımsız modülatörler

Sürüm 0.3.0 · 4 Kasım 2026 · Fazıl Irak

İndirmeler ve eşleşen kaynaklar GitHub Releases üzerinden sunulur. Kurulu DAW eklentileri otomatik değiştirilmez.

## Yapı

MODULATORS penceresinde OSC 1 PITCH, OSC 2 PITCH, CUTOFF, RESONANCE, PAN ve AMP sayfaları vardır. Her sayfa kendi Multi EG, LFO ve Step Sequencer ayarlarını saklar: toplam 18 bağımsız modülatör. Sayfa değiştirmek diğer hedefleri susturmaz; altı takım aynı anda çalışabilir. Her kaynak kendi Status alanından açılır. SuperSaw ve VA Stack tek osilatör bankası kullandığı için bu kiplerde OSC 2 Pitch'in etkisi yoktur.

Yerleşim, Rapture kılavuzunun 36–45. sayfalarındaki hedef başına bağımsız modülatör yaklaşımını izler: EG solda/ortada, LFO sağda, Step Sequencer altta. Model F'nin mevcut motoru ve zarf araçları korunmuştur; Rapture'ın tüm özelliklerinin birebir kopyası değildir. Örneğin Rapture'ın ayrı keytracking editörü ve kullanıcı WAV dosyasından LFO dalgası yüklemesi bu sürümün kapsamında değildir.

## Kontroller

Sayısal değerleri yukarı/aşağı sürükleyin veya fare tekerleği kullanın. Shift hassas ayar yapar. Kontrol/Alt tıklama varsayılana döndürür. Status ve diğer seçim alanlarında sol tıklama ileri, sağ tıklama geri gider. Üstteki hedef sekmeleri dışındaki parametreler etiket–değer satırlarıyla gösterilir.

Multi EG içindeki eski AD modu yeni panelde bulunmaz. Status OFF, o hedefin EG'sini devreden çıkarır; LFO ve Step ayrı çalışabilir. EG Depth, Duration, Once/Loop/Hold, Free/x1–x32 tempo seçimi, Fix Tempo, X/Y Grid, Snap, Curve, Node dalgası ve derinliği, sekiz dalgalı Ctrl+sürükle fırçası, Reset ve şekil Save/Load korunmuştur. Şekil dosyası seçili sayfanın EG'sine uygulanır. EG Brush, EG grafiğinin altında Node, Node depth ve Curve ile aynı satırdadır.

Yeni bankalar ve RESET EG, 0 → attack tepesi → decay → düz sustain → 0 şeklinde sade, beş noktalı bir EG çizimiyle başlar. Ek noktalar ekleyerek multistage şekiller kurabilirsiniz. Kayıtlı özel şekiller korunur. Bu bir başlangıç çizimidir; Playback zamanlama seçenekleri ayrıca ayarlanır.

Noktayı sürükleyin; çift tıklayarak nokta ekleyin/kaldırın. Kare tutamaklar segment eğrisini düzenler. Ctrl+sürükle EG Brush dalgasını çizer. Node dalgası seçimi fırçayı da ilgili dalgaya geçirir. En fazla 16 iç nokta bulunur.

LFO: Off/On/Shot; sekiz dalga, başlangıç fazı, 0–40 Hz, tempo senkronizasyonu, Delay, Fade ve Depth. Shot tek döngü çalar. Delay ve Fade nota başına uygulanır.

Step Sequencer: 2–128 adım; Off/On/Shot/Step; frekans, tempo senkronizasyonu, Smooth, Depth, 10/12/24 seviye snap ve Reset. Grafikte sürükleyerek adımları çizin. Ctrl ile orta değere sıfırlayın. Freq bütün desenin saniyedeki tekrar sayısıdır; tek adımın hızı değildir. Step modu her yeni notada bir sonraki adıma ilerler. Step → EG, adım değişiminde aynı hedefin EG'sini yeniden tetikler.

Ana MODULATORS düğmesi, kaynaklar kapalıyken PANIC ile aynı gri renktedir; herhangi bir kaynak açıkken turuncudur. Pencerede seçili hedef turuncu; seçili olmayan açık kaynaklı hedef logo beyazı; diğer hedefler gridir. Sıfır Depth, açık Status göstergesini kapatmaz. Renkler sayfada görünen Status alanlarından türetilir; eski presetlerin gizli AD yolu, üçü de OFF olan hedefi beyaza çevirmez.

## Ses yolu

OSC 1 Pitch, OSC 2 Pitch, cutoff, resonance ve pan için açık kaynakların etkileri kendi hedeflerinde toplanır. İki Pitch bankası yalnız kendi osilatörünün perdesini etkiler. Pitch derinliği ±4800 cent, cutoff ±4 oktav, resonance Model F Emphasis aralığı, pan sol/sağ denge aralığı üzerinden ölçeklenir. Rezonans ve pan geçerli motor sınırlarında tutulur.

AMP için EG, LFO ve Step kazançları çarpılır. Sıfır derinlik veya kapalı kaynak birim kazançtır. Bu, bir kaynağın sessiz evresinin diğer kaynak tarafından yeniden açılmasını önler.

## Yeni preamp

Eski Output knobu ve Full/Legacy düğmesi panelden kaldırılmıştır. Sağ üstte çerçevesiz GAIN ve TRIM etiket–değer alanları ile MIC/LINE ve IN anahtarları vardır. Değeri yukarı/aşağı sürükleyerek veya tekerlekle değiştirin. GAIN değeri turuncu, TRIM değeri logo beyazıdır; üst etiketler aynı hizada ve boyuttadır.

- GAIN: 0–36 dB giriş sürüşü; yükseldikçe analog renk ve doygunluk artar.
- TRIM: −60…+12 dB son çıkış seviyesi. GAIN ile seçilen doygunluk karakterini değiştirmeden seviyeyi ayarlar.
- LINE daha geniş headroom kullanır; MIC daha erken doygunluğa girer.
- IN kapalıyken renk/drive katmanı atlanır; TRIM çalışmaya devam eder.

Bu Model F'ye ait özgün bir renk katmanıdır; görsel referanstaki cihazın birebir devre modeli değildir. Asimetrik yumuşak doygunluk, hafif düşük frekans hafızası ve DC engelleme içerir. Nonlineer katmanda birinci dereceden antiderivative antialiasing kullanılır. Kontrol değişimleri 20 ms süzülür. Trim yükseltildiğinde DAW çıkışı yine aşırı seviyeye ulaşabilir; preamp bir çıkış limiter'i değildir.

Yeni örnekler ve fabrika sesleri yeni preamp yolunu kullanır. Eski oturum/presetler önceki Output ve Full/Legacy değerleriyle açılır; ilk preamp düzenlemesi yeni yola geçer. Eski parametreler yalnız bu kayıt uyumluluğu için korunur ve panelde görünmez.

## Eski kayıtlar

Eski host parametre kimlikleri ve sırası korunur; yeni bankalar sona eklenir. Eski preset/oturumlar açıldığında mevcut seslerini korumak için eski modülasyon yolu kullanılır. İlk yeni modülasyon düzenlemesinde yalnız panelde ON görünen eski Multi EG ilgili hedefe aktarılır ve bağımsız bankalar etkinleşir. Eski ortak Pitch zarfı, OSC 1 ve OSC 2 bankalarına birlikte aktarılır; bundan sonra ayrı düzenlenirler. Gizli eski AD yolu bu geçişte yeni EG’ye çevrilmez ve başka bir hedefin Status alanını açmaz. Yeni sistem etkinleştiğinde gizli AD yolu devreden çıkar; yalnız görünen/açık kaynaklar çalışır. Yeni paneli düzenlemeye kadar eski presetin ses yolu korunur. Değiştirmeden önce kendi presetinizin kopyasını saklayabilirsiniz.

## Doğrulama kapsamı

Yerel test sonuçları dağıtımın TEST-RESULTS.txt dosyasına yazılır. PNG önizlemeleri gerçek panel çizicisinden üretilir. Gerçek DAW içinde kullanıcı dinlemesi ve macOS/Linux çalışma doğrulaması ayrıca gereklidir.
