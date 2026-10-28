# F-EQ — Türkçe geliştirici ve hata müdahale rehberi

**Fazıl Irak · Son düzenleme: 22.10.2026**

F-EQ 0.1.11 Rust/nih-plug projesidir. Tilt, Bass Lift, ayrı Q, dik kesimler, analog/saturation ve çıkış spektrumu kaynakları korunarak taşındı.

## Derleme ve bağımlılıklar

Rust 1.97.1 + MSVC. Music/_Tools/Rust toolchain, _Tools/F-EQ-Cargo bu projeye ait paket önbelleği sağlar. Yeni build.ps1 eski ca-72 klasörüne ihtiyaç duymaz. `powershell -NoProfile -File build.ps1 -Test -Offline` mevcut çıktı adlarıyla yerel paket üretir. Kullanıcı kaynak yayımlamasını seçmedikçe -PackageSource gerekmez.

## Veri ve ses akışı

Host lib.rs process → params settings (smoothed fiziksel değerler) → dsp Engine/shelf + cuts → analog/saturation ve bypass karışımı → çıkış + spectrum Analyzer → editor/panel/window çizimi. GUI frekans konumu ile host normalized parametresi aynı dönüşüm değildir.

## Değişiklik ve hata dosyaları

src/params.rs: id/aralık/varsayılan/birim. dsp.rs: biquad katsayı, ayrı shelf Q ve örnek hafızası. cuts.rs: Butterworth kademeleri/eğim değişimi. analog.rs: tolerans/gürültü; saturation.rs: bağımsız nonlinear işlem. spectrum.rs: FFT/tampon/gösterim. panel.rs: halka geometrisi ve ölçek; editor.rs/window.rs: native olay/focus/capture ve about pencere kapatma.

## Belirtiyi yorumlama

Tilt grafiği/ses uyuşmazlığı: coefficients/response ile Engine sample’ın aynı fiziksel değerlerini kullandığını kontrol edin. Bas değişince tiz etkileniyorsa bass_q ve tilt q alanlarını ayırın. Eğim değişiminde klik: cuts configure/smooth ve eski kademe hafızası. Bypass gürültüsü: analog/saturation ile dry eşleşmesi. Spektrum ters stereo fazda kayboluyorsa Analyzer’ın kanalları nasıl topladığını ve FFT skalasını inceleyin.

## Testler

`build.ps1 -Test -Offline` unit ve CLAP host testlerini çalıştırır, preview ve Windows kitaplık/standalone çıktısını üretir. tests/clap_host.rs gerçek plug-in host çağrıları, automation, bypass ve session roundtrip kontrolüdür. Kesim testleri yalnız çizilmiş eğri değil işlenmiş audio cevabını da ölçer.

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

13 uygulama/test kaynak dosyasına dosya görevi ve fonksiyon bazında Türkçe bakım açıklamaları eklendi. C++/Rust yorum dışı token dizisi özgün kaynakla denetlendi. Üçüncü taraf upstream/Dependencies/third_party telif ve lisansları korunmuştur. Bakım üst bilgisi özgün telif sahibini değiştirmez. JSON veri dosyalarına geçersiz yorum konmadı; envanterde tarih/sahiplik kaydı tutulur.

Bu açıklama çalışması kapsamlı bir güvenlik veya tüm DAW uyumluluk sertifikası değildir. Yeni derleme mevcut yayımlanmış Release/ZIP’i veya web sitesi indirmesini otomatik değiştirmez. Eski kaynak/dağıtım materyalleri Music/_Arsiv altında tutulur; build/target gibi yeniden üretilebilen önbellekler aktif kaynak ağacına taşınmaz.

## Dosya ve fonksiyon haritası

### src/analog.rs

F-EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır.

Fonksiyonlar: `default`, `unit`, `noise`, `change`, `smooth`, `reset`, `noise_and_variation_match_original_ranges`.

### src/cuts.rs

Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular.

Fonksiyonlar: `default`, `reset`, `configure`, `smooth`, `sample`, `section`, `magnitude`, `all_orders_have_butterworth_corner_and_expected_rejection`.

### src/dsp.rs

F-EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.

Fonksiyonlar: `default`, `default`, `normalized`, `shelf`, `cut`, `magnitude`, `complex`, `coefficients`, `shelf_coefficients`, `response`, `tick`, `default`, `prepare`, `set`, `rebuild`, `sample`, `neutral_is_transparent`, `tilt_has_opposite_shelves_and_fixed_pivot`, `bass_and_cuts`, `extremes_stable_and_stereo_independent`, `bypass_becomes_exact_dry`, `tilt_and_bass_q_control_separate_shelves`, `analog_noise_is_subtle_and_bypass_removes_it_and_saturation`, `actual_audio_measures_96_db_per_octave_and_click_free_switching`.

### src/editor.rs

Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.

Fonksiyonlar: `drop`, `new`, `spawn`, `size`, `set_scale_factor`, `param_value_changed`, `param_modulation_changed`, `param_values_changed`, `host_key_down`, `new`, `finish`, `set_about`, `set`, `draw`, `on_frame`, `on_event`, `dismiss_key`, `vst3_host_keys_only_close_an_open_about`, `native_escape_enter_and_keypad_enter_close_but_releases_do_not`.

### src/lib.rs

F-EQ host girişini, parametre/durum kaydını ve DSP callback yaşam döngüsünü bağlar.

Fonksiyonlar: `default`, `filter_state`, `params`, `editor`, `initialize`, `reset`, `process`, `legacy_state_explicitly_resets_added_controls`.

### src/panel.rs

Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.

Fonksiyonlar: `logo_hit`, `about_hit`, `about_close_hit`, `frequency_limits`, `frequency_position`, `knob_position`, `default_position`, `host_normalized`, `hit`, `power_hit`, `analog_hit`, `slope_hit`, `value`, `text`, `center`, `pointer`, `ticks`, `knob`, `main_knob`, `button`, `graph`, `svg`, `svg_with_spectrum`, `options`, `with_about`, `render`, `frequency_travel_matches_endpoints_and_actual_hz`, `concentric_controls_are_independent_hit_regions`, `both_slope_rows_are_independently_selectable`.

### src/params.rs

Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.

Fonksiyonlar: `default`, `db`, `hz`, `knobs`, `settings`.

### src/saturation.rs

F-EQ doygunluk eğrisi ve antitürev tabanlı sayısal işleme adımıdır.

Fonksiyonlar: `sample`, `integral`, `zero_is_exact_and_drive_softens_peaks`, `generates_third_harmonic`.

### src/spectrum.rs

F-EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir.

Fonksiyonlar: `default`, `fft`, `push`, `cursor`, `reset`, `read`, `sine_frequency_and_level_survive_opposite_stereo_polarity`, `silence_and_reset_have_no_spurious_spectrum`.

### src/window.rs

Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.

Fonksiyonlar: `can_open`, `display`, `display`, `raw_window_handle`, `new`, `drop`, `hwnd`, `take_keys`, `give_keys_back`, `lost_capture`, `release_capture`, `display_handle`, `window_handle`, `target`, `guarded`, `new`, `resize`, `present`, `show`.

### tests/clap_host.rs

Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.

Fonksiyonlar: `id`, `rescan`, `clear`, `request`, `host_extension`, `in_size`, `in_get`, `out_push`, `write`, `read`, `new`, `extension`, `process`, `value`, `save`, `load`, `drop`, `clap_audio_automation_bypass_and_session_roundtrip`.

### examples/preview.rs

preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `main`.

### examples/standalone.rs

standalone geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `main`.


## C# bilen geliştirici için Rust kodunu okuma

- `fn` metot/fonksiyon, `struct` veri yapısı; `impl Tip` içindeki fonksiyonlar o tipin işlemleridir.
- `&self` nesneyi ödünç okuyabilir; `&mut self` durumu değiştirebilir. `Arc` paylaşılan sahipliktir; `Mutex` bekleme maliyeti taşıyabilir. `Drop` kaynak bırakma aşamasıdır.
- `Option` yok/var, `Result` başarı/hata taşır. `?` hatayı çağırana iletir; `unwrap` beklenmedik hata halinde panic oluşturabilir. Audio callback içine yeni panic yolu eklemeyin.
- `unsafe` C#’taki güvenli tip/ömür denetimlerinden çıkılan bir sınır gibi ele alınır: pointer sahibi, null, tampon boyu ve thread ömrü birlikte incelenir. Mevcut güvenlik yorumunu silmeyin.
- `#[cfg(test)]` yalnız test derlemesinde; `#[test]` test girişidir. `cargo test -p PAKET --release --locked --offline -- --list` test adlarını listeler. Ad filtresiyle tek senaryoyu seçerek başlayabilirsiniz.
- Debug sembolleri için ayrı `cargo build` dev derlemesi kullanın; mevcut release ayarları sembolleri azaltabilir. Rust destekli debugger ile yerel exe/host sürecine bağlanın. `RUST_BACKTRACE=1` bir terminal testindeki panic çağrı yolunu gösterebilir; GUI/host içine dosya logu eklemek yerine testte tekrar üretin.

Türkçe açıklamayı değiştirdiğiniz gerçek tarihte üst bilgiyi de güncelleyin. Önce küçük bir branch/kopya alın; kaynakların yerel taşınması GitHub’a otomatik gönderim değildir.
