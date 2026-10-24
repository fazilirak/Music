# Model-F — Türkçe geliştirici ve hata müdahale rehberi

**Fazıl Irak · Son düzenleme: 22.10.2026**

Güncel Model F 0.1.9 için ca-72-0.1.3 içindeki Model-F çalışma alanı taşındı. Parent CA72 kaynakları da eski proje arşivinde korunur; bu çalışma alanı bağımsız Cargo workspace’tir.

## Derleme ve bağımlılıklar

Rust 1.97.1 ve MSVC C++ araçları. Music/_Tools/Rust toolchain ve Cargo önbelleğiyle eski parent .tools yoluna bağımlılık kaldırıldı. `powershell -NoProfile -File build.ps1 -Test -Offline` test/preview ve Windows VST3/CLAP/Standalone üretir; -PackageSource ayrıca kaynak ZIP’i üretir, kendiliğinden paylaşmaz. third_party yamalı çerçeve kaynakları ve lisansları korunur.

## Veri ve ses akışı

Host lib.rs → params/Controls → engine nota/MIDI ve kip seçimi → core Voice → VCO/contour/nonlinear ladder VCF/VCA/noise → örnek çıkışı. Arpeggiator ve MSEG host örnek zamanı/tempo olaylarıyla akışı değiştirir. SuperSaw sabit yedi osilatör dağılımı ve ayrı voice/filtre/contour davranışıyla izlenir.

## Değişiklik ve hata dosyaları

crates/model-f/src/lib.rs: host/FFI/parametre. params.rs: kimlik ve aralık. engine.rs: mono/poly/SuperSaw, MIDI, Panic ve voice hayatı. arp.rs/mseg.rs: zamanlı kontrol. presets.rs: JSON/version/capture/apply. editor.rs/mseg_ui.rs/panel.rs/window.rs: GUI/focus/capture. model-f-core: vco/vcf/vca/contour/mna sayısal devre. model-f-rt: gerçek zamanlı araçlar.

## Belirtiyi yorumlama

NaN/çatlama: devre solver yakınsaması, internal sample rate, state reset ve mna/ulp/unconverged sayaçları. Çok CPU: etkin/releasing voice sayısı, SuperSaw katmanları, threaded görev/rt allocation. Asılı ses: MIDI/HOLD/MSEG/contour ve Panic istek sayacı; GUI klavye bırakmasıyla host yeni notalarını ayırın. Eğri yanlış: mseg_ui fiziksel koordinatı ile mseg zaman/eğri hesabını ayrı sınayın.

## Testler

Önce `cargo test -p model-f --release --locked --offline`; sonra ilgili core integration testini seçin (ör. `cargo test -p model-f-core --test supersaw --release --locked --offline`). model-f-core rt_alloc, threaded, vcf_jacobian, mna ve voice testleri farklı riskleri kontrol eder. Tüm workspace’in optimize edilmemiş testleri çok uzun sürebilir; release profilini kullanın.

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

70 uygulama/test kaynak dosyasına dosya görevi ve fonksiyon bazında Türkçe bakım açıklamaları eklendi. C++/Rust yorum dışı token dizisi özgün kaynakla denetlendi. Üçüncü taraf upstream/Dependencies/third_party telif ve lisansları korunmuştur. Bakım üst bilgisi özgün telif sahibini değiştirmez. JSON veri dosyalarına geçersiz yorum konmadı; envanterde tarih/sahiplik kaydı tutulur.

Bu açıklama çalışması kapsamlı bir güvenlik veya tüm DAW uyumluluk sertifikası değildir. Yeni derleme mevcut yayımlanmış Release/ZIP’i veya web sitesi indirmesini otomatik değiştirmez. Eski kaynak/dağıtım materyalleri Music/_Arsiv altında tutulur; build/target gibi yeniden üretilebilen önbellekler aktif kaynak ağacına taşınmaz.

## Dosya ve fonksiyon haritası

### crates/model-f/examples/model-f.rs

model-f geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `main`.

### crates/model-f/examples/overload-audition.rs

overload-audition geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `wav`, `render`, `bands`, `main`.

### crates/model-f/examples/preview.rs

preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `main`.

### crates/model-f/examples/render-bank.rs

render-bank geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `plugin_api`, `execute`, `set_latency_samples`, `set_current_voice_capacity`, `wav`, `main`.

### crates/model-f/examples/supersaw-load.rs

supersaw-load geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.

Fonksiyonlar: `main`.

### crates/model-f/src/arp.rs

Basılı MIDI notaları, host/dahili tempo ve gate durumundan örnek zamanlı arpej olayları üretir.

Fonksiyonlar: `default`, `configure`, `note`, `stop`, `clear`, `revoice`, `boundary`, `next_in`, `advance`, `arp`, `up_down_and_sample_gates`, `latch_replaces_chord_and_channel_release_is_counted`, `octave_expansion_clamps_midi_and_deduplicates`, `disabling_hold_with_no_physical_keys_releases_the_gate`.

### crates/model-f/src/character.rs

Analog karakter farklarını, rastgele toleransları, gain ve pan dağılımını hesaplar.

Fonksiyonlar: `new`, `uniform`, `normal`, `knob`, `offsets`, `gains`, `pan_gains`.

### crates/model-f/src/editor.rs

Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.

Fonksiyonlar: `drop`, `new`, `spawn`, `size`, `set_scale_factor`, `param_value_changed`, `param_modulation_changed`, `param_values_changed`, `env_finish`, `env_apply`, `envelope_file`, `new`, `reload`, `refresh_rows`, `load`, `step_preset`, `end_drag`, `set_normalized`, `selector`, `publish_keys`, `release_keys`, `save_user`, `import`, `export`, `click`, `key`, `draw`, `on_frame`, `on_event`.

### crates/model-f/src/engine.rs

Nota/voice, MIDI, çalışma kipleri, arpej, MSEG, filtre ve ses render akışını koordine eder.

Fonksiyonlar: `lock_trim`, `default`, `new`, `step`, `lift`, `rest`, `hand`, `ran`, `play`, `default`, `mend`, `mend_until`, `make`, `fill`, `default`, `with_voice`, `new`, `prepare`, `start_workers`, `stop_workers`, `follow_poly`, `crew`, `take_serving`, `spares`, `take_mending`, `workers`, `set_deadline`, `set`, `one_note`, `event`, `all_sound_off`, `settle`, `restore`, `poly_on`, `voices`, `release_all`, `lift_all`, `tick`, `tick_stereo`, `render`, `run`, `bring_up`, `faded`, `fade_on`, `mix`, `sample`, `voice_rate`, `end_block`, `lamp`, `sounding`, `wheels`, `midi_wheels`, `panel`, `set_in_loop`, `apply`, `voice_panel`, `default_workers`, `to_array`, `from_array`, `jacks`, `realtime_voices`, `measure_voices`, `voice_seed`, `entropy_panel`, `output_gain`, `silent_volume_stage_does_not_discard_a_voice_before_amp_release`, `supersaw_plays_8_notes_and_steals_released_before_held`, `four_voices`, `a_voice_still_held_sits_out_and_nothing_waits_for_it`, `a_worker_late_past_the_deadline_is_not_waited_for`, `a_voice_that_misses_a_run_is_not_freed_for_it`, `a_drop_for_a_run_not_played_is_kept_for_the_next`, `a_voice_gone_non_finite_is_put_back_to_rest`, `workers_running_for_the_voices_are_kept`, `a_mend_stops_before_its_next_voice`, `planned`, `switch`, `serve`, `an_instance_holds_workers_only_while_poly_is_on`, `an_instance_with_poly_off_leaves_its_workers_to_one_with_it_on`, `a_pool_for_voices_gone_is_stopped`, `a_new_seed_draws_the_characters_again`.

### crates/model-f/src/helper.rs

Ana uygulamayı kilitlemeden yardımcı iletişim/iş ve kapanış durumlarını yönetir.

Fonksiyonlar: `start`, `ask`, `drop`, `help`, `hold`, `hold`, `held`, `ended`, `watch`, `until`, `a_helper_dropped_mid_round_is_waited_for`.

### crates/model-f/src/lib.rs

Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.

Fonksiyonlar: `default`, `drop`, `panic`, `parameters`, `event`, `filter_state`, `params`, `editor`, `initialize`, `deactivate`, `reset`, `process`, `plugin_api`, `execute`, `set_latency_samples`, `set_current_voice_capacity`, `block`, `quick_panel_panic_survives_mouse_up_and_silences_every_mode_and_latched_arp`.

### crates/model-f/src/mseg.rs

Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.

Fonksiyonlar: `legacy_target`, `default`, `from_params`, `shape`, `default`, `fraction`, `bend_from_fraction`, `wave`, `new`, `value`, `default`, `configure`, `reset`, `tick`, `route`, `saved_id`, `capture`, `validate`, `apply`, `save`, `read`, `machine_shape_timing_playback_and_release`, `all_waves_endpoints_curves_and_crossed_slots_stay_bounded`, `routing_and_envelope_files_do_not_change_other_synth_controls`, `zero_depth_keeps_note_timing`, `output_gain_depth_and_polarity_are_bounded`.

### crates/model-f/src/mseg_ui.rs

Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.

Fonksiyonlar: `point_id`, `node_id`, `value`, `xy`, `pixel`, `snap`, `curve_handle`, `move_node`, `down`, `motion`, `key`, `paint_cell`, `draw`, `knob`, `wheel`, `apply`, `graph_add_drag_delete_and_curve_gestures`, `wave_brush_populates_stable_slots_and_preserves_outside_cells`, `overlay_renders_at_both_scales_with_spaced_controls`, `texts`.

### crates/model-f/src/panel.rs

Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.

Fonksiyonlar: `in_rect`, `hit`, `escape`, `text`, `button`, `section`, `value`, `open_about`, `about_click`, `svg`, `render`, `about_opens_and_closes_without_changing_synth_parameters`, `rendered_labels_keep_clearance_from_text_and_controls`, `texts`, `overlaps`, `controls_cover_every_parameter_and_do_not_overlap`, `panel_and_browser_render_at_two_scales`.

### crates/model-f/src/params.rs

Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.

Fonksiyonlar: `envelope_slots`, `controls`, `mono_is_default_and_uses_normal_monophonic_oscillators`, `existing_mode_numbers_keep_their_dsp_behavior`, `overload_feedback`.

### crates/model-f/src/pool.rs

Ses işçisi/görev sahipliği, iş alma-verme ve iş parçacığı koordinasyonu için yerel havuz durumunu yönetir.

Fonksiyonlar: `play`, `new`, `held`, `claim`, `give_back`, `word`, `current`, `take`, `fmt`, `start`, `start_from`, `workers`, `block`, `play`, `offer`, `wake`, `close`, `drop`, `fmt`, `default`, `lock`, `new`, `with_budget`, `plan`, `clear`, `start`, `serve`, `tidy`, `deliver`, `follow`, `work`, `drop`, `play`, `fakes`, `mix`, `until`, `a_worker_late_for_a_run_over_leaves_its_voice_alone`, `a_late_worker_does_not_unmark_a_later_runs_voice`, `past_the_deadline_the_callers_thread_plays_every_voice`, `the_workers_take_the_callers_flush_bits`, `workers_come_out_of_one_budget`, `a_worker_not_started_stops_the_others`, `a_crew_holds_workers_only_while_poly_is_on`, `poly_switched_quickly_keeps_no_worker`, `a_new_plan_stops_the_old_ones_pools`.

### crates/model-f/src/presets.rs

Preset sürüm geçişi, doğrulama, capture/apply, fabrika/kullanıcı dosyaları ve arama akışıdır.

Fonksiyonlar: `upgrade`, `capture`, `validate`, `apply`, `differs`, `factory`, `directory`, `read`, `save`, `atomic_write`, `save_user`, `user`, `factory_has_160_distinct_sounds_and_40_multi_stage_sounds`, `roundtrip_and_reject_bad_file_before_applying`, `legacy_27_control_presets_remain_compatible`, `previous_160_control_presets_default_to_dry_feedback`, `old_targets_upgrade_without_reinterpreting_oscillator_two_as_volume`.

### crates/model-f/src/window.rs

Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.

Fonksiyonlar: `can_open`, `display`, `display`, `raw_window_handle`, `new`, `drop`, `hwnd`, `take_keys`, `give_keys_back`, `lost_capture`, `release_capture`, `display_handle`, `window_handle`, `target`, `guarded`, `new`, `resize`, `present`, `show`.

### crates/model-f/tests/clap_host.rs

clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `id`, `rescan`, `clear`, `request`, `host_extension`, `in_size`, `in_get`, `out_push`, `write`, `read`, `new`, `extension`, `process`, `value`, `save`, `load`, `drop`, `json_of`, `clap_automation_and_full_session_roundtrip`, `clap_supersaw_midi_is_sample_timed_and_can_be_silenced`, `legacy_sessions_reset_new_modulation_even_in_an_existing_instance`, `legacy_six_target_sessions_migrate_to_the_three_targets`.

### crates/model-f/tests/mseg_audio.rs

mseg_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `sound`, `every_destination_changes_audible_circuit_output`.

### crates/model-f/tests/overload_audio.rs

overload_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `params`, `sound`, `feedback_changes_sound_and_lights_the_real_detector`, `attack_sound`, `overload_does_not_blow_up_resonant_note_attacks`, `zero_overload_preserves_the_original_dry_path`.

### crates/model-f/tests/realtime.rs

realtime regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `alloc`, `dealloc`, `new_modes_do_not_allocate_or_free_on_the_audio_thread`.

### crates/model-f-core/src/a440.rs

Referans A440 sinyalinin açma-kapama ve örnek üretimini uygular.

Fonksiyonlar: `new`, `is_on`, `tick`.

### crates/model-f-core/src/a440_table.rs

Referans sinyal için önceden üretilmiş tablo verileridir; indeks ve örnekleme ölçeği korunmalıdır.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### crates/model-f-core/src/contour.rs

Analog contour devresi, tetik ve zaman sabitlerini sayısal devre modeliyle hesaplar.

Fonksiyonlar: `default`, `default`, `root`, `root_slope`, `root_dir`, `grounded_npn`, `grounded_npn_with`, `grounded_npn_slopes`, `set_quality`, `set_step`, `new`, `devices_and_diodes`, `settle`, `setup`, `peak`, `q6_emitter`, `follow`, `tick`, `capacitors`, `peaks`, `trigger_nodes`.

### crates/model-f-core/src/devices.rs

Transistör/diyot benzeri elemanların akım, türev ve sıcaklık davranışlarının denklemleridir.

Fonksiyonlar: `vt`, `degenerated`, `degenerated_from`, `tanh`, `degenerated_warm`, `degenerated_warm_to`, `law`, `at`, `current`, `series_junction`, `series_junction_from`, `junction_capacitance`, `new`, `at`, `at`, `base_law`, `transport`, `base_current`, `base_slope`, `transport_slope`, `junction_voltage`, `gm_factor`, `currents`, `currents_d`, `at`, `solve_junctions`, `solve_tail_pair`, `solve3`, `solve_junctions_limited`.

### crates/model-f-core/src/expo.rs

Osilatör frekans kontrolünün üstel dönüştürücü devre/sayısal çözümünü uygular.

Fonksiyonlar: `default`, `v_sum`, `drive_u`, `input_current`, `collector_current`, `solve`, `solve_current`, `new`, `lookup`, `the_converter_table_matches_the_solve`.

### crates/model-f-core/src/fast.rs

Ses motorundaki hızlı matematik yaklaşımlarını referans fonksiyonlarla karşılaştırır.

Fonksiyonlar: `exp`, `tanh`, `they_follow_libm`.

### crates/model-f-core/src/filter_cal.rs

Filtre devresi kalibrasyonunu, frekans/rezonans ölçüm ve eşleştirme adımlarını yürütür.

Fonksiyonlar: `expo`, `circuit`, `inputs`, `run`, `frequency`, `oscillation`, `regenerates`, `bisect`, `key_volts`, `folkman`.

### crates/model-f-core/src/keyboard.rs

Klavye kontrol voltajı, nota önceliği ve ilgili devre yükünün çözümünü uygular.

Fonksiyonlar: `default`, `of`, `tolerance`, `substeps`, `source_current`, `string_current`, `source_thevenin`, `new`, `string_current`, `key_voltage`, `circuit`, `set_quality`, `set_rate`, `out`, `probe`, `set_glide`, `set_load`, `contacts`, `apply`, `flush`, `step`, `halve`, `tick`, `settle`, `static_out`, `solver_parts`.

### crates/model-f-core/src/lib.rs

Model F devre çekirdeğinin modül/kamuya açık tür sözleşmesini bir araya getirir.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### crates/model-f-core/src/linear.rs

Lineer devre matris çözümü, frekans cevabı, ayrık durum ve taşıma adımlarını hesaplar.

Fonksiyonlar: `solve_complex`, `invert_into`, `response`, `discrete`, `rebuild`, `take_state`, `carry`, `step`, `x`, `response`.

### crates/model-f-core/src/mna.rs

Modified nodal analysis ile devre elemanları/matris/türev ve nonlinear yakınsama adımlarını uygular.

Fonksiyonlar: `intrinsic`, `forward`, `channel`, `charge_near`, `consts`, `beyond_consts`, `beyond`, `charge`, `clone`, `new`, `flatten`, `pnjlim`, `diode_charge`, `diode_charge_e`, `diffusion_charge`, `vcrit`, `fetlim`, `new`, `node`, `add_bjt`, `add_jfet`, `add`, `v`, `set`, `set_theta`, `charge_current`, `set_resistance`, `free`, `stamp_nodes`, `build_slots`, `stamp_at`, `charge`, `load`, `linear_solve`, `replay_flat`, `back_flat`, `newton`, `dc`, `step`, `accept`, `residual_and_jacobian`, `small_signal`, `solved_index`, `size`, `extent`, `linear_nodes`, `components`, `find`, `kind`, `census`, `time_parts`, `reference`, `the_replayed_elimination_matches_the_reference_to_the_bit`.

### crates/model-f-core/src/modulation.rs

Mod wheel/pitch ve modülasyon kaynaklarının devre yükü/voltaj katkılarını hesaplar.

Fonksiyonlar: `pitch_wheel_volts`, `mod_wheel_r`, `circuit`, `transfer`, `new`, `mix`, `source_loads`, `line`.

### crates/model-f-core/src/noise.rs

Noise devre kaynağının yoğunluk, yük ve örnek üretim durumunu uygular.

Fonksiyonlar: `default`, `circuit`, `circuit_with`, `open`, `new`, `model_with`, `set_loads`, `apply_loads`, `set_density`, `set_quality`, `reseed`, `density`, `model_of`, `tick`, `latency`, `new`, `next_u32`, `uniform`, `next`.

### crates/model-f-core/src/overload.rs

Giriş/mikser aşırı sürüş karakterini hafızalı devre/sayısal modelle uygular.

Fonksiyonlar: `new`, `gate`, `mixer`, `tick`, `silence_and_new_note_return_have_no_startup_impulse`, `third_harmonic`, `mixer_adds_progressively_audible_harmonics_at_equal_gain`.

### crates/model-f-core/src/preamp.rs

Ön yükselteç kazanç, kırpma ve devre yükünü sayısal olarak uygular.

Fonksiyonlar: `default`, `high`, `clip`, `tick`, `new`, `reset`, `high`, `tick`, `circuits`, `set_quality`, `new`, `set_source_resistance`, `set_switch`, `step`, `latency`, `set_delayed`, `tick`, `outputs`, `the_plain_preamplifier_against_the_circuit`, `the_lamp_after_the_drive_stops`, `the_plain_preamplifier_harmonics`, `the_plain_preamplifier_follows_the_circuit`.

### crates/model-f-core/src/prof.rs

Çevrimdışı/geliştirme performans ölçümlerinin zaman ve bölüm sayaçlarını taşır.

Fonksiyonlar: `start`, `lap`, `take`, `start`, `lap`, `take`.

### crates/model-f-core/src/resample.rs

Örnekleme oranı dönüşümünün FIR/halfband katsayı, gecikme ve örnek geçmişini uygular.

Fonksiyonlar: `step`, `impulse`, `push`, `bessel_i0`, `halfband_taps`, `new`, `sparse`, `sum`, `delay`, `push`, `filter`, `filter_stuffed_zero`, `reset`, `new`, `sparse`, `with`, `push`, `delay`, `reset`, `new`, `sparse`, `with`, `factor`, `push`, `delay`, `reset`, `stages_len`.

### crates/model-f-core/src/revsaw.rs

Saw dalgasının devre şarj/boşalma ve kalite/adım durumunu hesaplar.

Fonksiyonlar: `default`, `new`, `set_quality`, `r_out`, `tick`, `charge`, `tick_plain`, `the_plain_stage_follows_the_circuit`.

### crates/model-f-core/src/shared.rs

Devre/voice tarafından paylaşılan salt okunur veya açık sahiplikli parametre verilerini taşır.

Fonksiyonlar: `new`, `get`, `default`.

### crates/model-f-core/src/supersaw.rs

Yedi osilatörün frekans dağılımı ve detune eğrisini uygular; osilatör sayısı sabittir.

Fonksiyonlar: `detune_curve`, `new`, `tick`, `resets`, `measured_nonlinear_detune_has_the_jp_knee`.

### crates/model-f-core/src/tables.rs

Devre modellerinin önceden hesaplanan yaklaşım/kalibrasyon tablolarıdır; keyfi yeniden ölçeklemeyin.

Veri/modül bildirimi; yürütme fonksiyonu içermez.

### crates/model-f-core/src/threaded.rs

Ses motorunun iş parçacığı dağıtımı, bekleme ve görev koordinasyonunu uygular.

Fonksiyonlar: `new`, `snooze`, `waited`, `run`, `run`, `run`, `run`, `run`, `drive`, `feed`, `given`, `fmt`, `spawn`, `begin`, `feed`, `take`, `finish`, `drop`, `work`, `drop`, `new`, `following`, `build`, `note`, `passes`, `overload`, `take_overload_peak`, `process`, `process_in_turn`.

### crates/model-f-core/src/tuning.rs

Nota/kontrol voltajından osilatör giriş, frekans ve kalibrasyon değerlerini hesaplar.

Fonksiyonlar: `octaves_below_2`, `default`, `osc1_inputs`, `osc1_inputs_at`, `freq_pot`, `osc3_control`, `apply`, `osc_drive`, `osc_drive_with`, `measure_hz`, `hz`, `range_and_scale`, `secant`, `folkman_1973`, `folkman_1973_keys`, `folkman_1973_buses`, `folkman_1973_osc`, `folkman_1973_osc_buses`.

### crates/model-f-core/src/ulp.rs

Sayısal yaklaşım/yuvarlama sapması gözlemlerini ve matematik yönlendirmesini yönetir.

Fonksiyonlar: `set`, `nudged`, `nudged`, `note`, `take`, `calls`, `new`, `drop`, `exp`, `tanh_in_tick`, `pow`, `potato_scopes_restore_libm`.

### crates/model-f-core/src/unconverged.rs

Nonlinear devre çözümünde yakınsamayan adımların gözlem sayaçlarını taşır.

Fonksiyonlar: `note`, `take`.

### crates/model-f-core/src/vca.rs

Voltaj kontrollü yükselteç devresini, kalibrasyon tablosunu ve kazanç/çıkış örneğini hesaplar.

Fonksiyonlar: `default`, `calibrated`, `lerp`, `pair_bias`, `lerp`, `to_array`, `from_array`, `place`, `lerp_entry`, `for_circuit`, `make`, `lookup`, `new`, `settle`, `chain_equilibrium`, `prepare_potato`, `solve_bias`, `fast_bias`, `path`, `rest_output`, `static_output`, `tick`, `drive`, `control`, `signal`, `step_chain`, `probe`, `chain`, `bias`, `the_bias_under_a_slow_sweep_up_and_down`, `plain_signal_follows_the_full_one`, `drive_table_under_a_sweep`, `drive_table_matches_the_solve`, `vca_parts`.

### crates/model-f-core/src/vcf.rs

Nonlinear ladder filtre devresini, yük/rezonans ve sayısal çözüm adımlarını uygular.

Fonksiyonlar: `default`, `root`, `current`, `tail_node`, `q28`, `at_node`, `solve`, `node`, `shared`, `new`, `current`, `current_and_node`, `default`, `lerp`, `bias`, `eval`, `eval_from`, `eval_full`, `eval_sparse`, `plain_pair`, `solve_loop`, `new`, `prepare`, `set_quality`, `table_bias`, `set_oversample`, `reset`, `latency`, `state`, `tick`, `drive`, `step`, `table_hermite`, `solve`.

### crates/model-f-core/src/vco.rs

Devre osilatörünün faz, dalga, oversampling, reset ve gecikme akışını yönetir.

Fonksiyonlar: `default`, `delay_at`, `default`, `triangle`, `new`, `prepare`, `set_oversample`, `last_reset`, `latency`, `reset`, `ramp`, `tick`, `timing_current`, `tick_current`, `step`, `do_reset`.

### crates/model-f-core/src/voice/loop_lab.rs

Voice geri besleme döngüsünü çevrimdışı karşılaştırıp yük/devre varyantlarını ölçer.

Fonksiyonlar: `panel`, `record`, `tick`, `circuit`, `plain`, `run`, `compare`, `the_preamplifiers_input_in_the_loop`, `the_circuit_driven_hard`, `band_errors`, `the_models_open_loop`.

### crates/model-f-core/src/voice.rs

Bir notanın osilatör/filtre/VCA/contour/noise devrelerini tek örnek akışında birleştirir.

Fonksiyonlar: `width`, `default`, `channel_load`, `noise_loads`, `calibrate_noise`, `audio_taper`, `time_pot`, `emphasis_r14`, `channel`, `waveform_source`, `glide_r`, `keyboard_load`, `feedback_law`, `get`, `ext_volume`, `new`, `prototype`, `set_seed`, `rate`, `envelopes`, `probe`, `probe_pitch`, `probe_resets`, `supersaw_resets`, `probe_contours`, `hold_control_quality`, `tunings`, `filter_probe`, `noise_density`, `revsaw`, `keyboard`, `note`, `tick`, `preamp`, `preamp_failed`, `ext_probe`, `overload`, `take_overload_peak`, `into_parts`, `tick_in`, `tick_jacks`, `tick_at`, `contour_panel`, `note`, `step`, `reserve`, `held`, `tick`, `beyond`, `keyboard_every`, `oversampling`, `contour_every`, `tick`, `drive`, `tick`, `overload`, `take_overload_peak`, `tick`, `tick`, `lamp`, `tick`, `tick`.

### crates/model-f-core/tests/a440.rs

a440 regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `the_a440_plays_its_circuits_waveform_at_440_hz`.

### crates/model-f-core/tests/entropy.rs

entropy regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `frequency`, `cents`, `an_oscillators_detune_is_its_cents`, `the_cutoffs_offset_is_its_octaves`.

### crates/model-f-core/tests/expo_table.rs

expo_table regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `table_matches_the_circuit_solution`.

### crates/model-f-core/tests/feedback.rs

feedback regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `play`, `above_1k`, `feedback_is_not_heard_with_the_external_input_off`, `feedback_overdrives_the_preamplifier_and_the_mixer`, `feedback_at_its_ends_stays_within_the_rails`, `the_knob_is_tapered`.

### crates/model-f-core/tests/filter_cal.rs

filter_cal regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `filter_calibration`.

### crates/model-f-core/tests/midi_range.rs

midi_range regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `frequency`, `cents`, `sweep`, `every_midi_note_continues_the_keyboards_scale`, `the_lowest_key_held_sounds_over_all_128`, `glide_crosses_the_strings_ends`, `the_oscillators_limits_at_the_ranges_ends`, `the_converter_is_held_at_its_ceiling_far_above_the_keys`.

### crates/model-f-core/tests/mna.rs

mna regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `every_part`, `jacobian_matches_finite_differences`, `a_diode_and_resistor_match_their_closed_form`, `an_rc_step_is_backward_euler`.

### crates/model-f-core/tests/noise.rs

noise regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `changed_loads_match_the_circuit_solved_with_them`, `potatos_noise_model_follows_the_circuits`.

### crates/model-f-core/tests/one_cpu.rs

one_cpu regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `start`, `the_voice_goes_on_with_all_its_threads_on_one_cpu`.

### crates/model-f-core/tests/resample_factors.rs

resample_factors regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `supported_factors_build`, `decimator_refuses_sixteen`, `interpolator_refuses_three`, `sparse_halfbands_match_the_full_ones`.

### crates/model-f-core/tests/retrigger.rs

retrigger regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `rise`, `the_contours_retrigger_after_the_trigger_has_been_open_twelve_ms`.

### crates/model-f-core/tests/rt_alloc.rs

rt_alloc regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `alloc`, `dealloc`, `the_voice_does_not_allocate_while_it_plays`, `the_voice_on_threads_does_not_allocate_while_it_plays`.

### crates/model-f-core/tests/supersaw.rs

supersaw regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `seven_circuit_oscillators_follow_measured_frequency_distribution`, `detune_zero_and_mode_changes_remain_finite`.

### crates/model-f-core/tests/threaded.rs

threaded regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `noop`, `threads_give_the_same_samples_as_one`, `a_stopped_worker_leaves_silence_not_a_wait`, `stop`, `stop_contours`, `stop_bias`.

### crates/model-f-core/tests/vcf_jacobian.rs

vcf_jacobian regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `filter_jacobian_matches_finite_differences`, `check`, `series_junction_balances_its_resistor`.

### crates/model-f-core/tests/voice.rs

voice regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.

Fonksiyonlar: `rms`, `pitch`, `cents`, `a_key_sounds_at_its_pitch_and_releases`, `the_lowest_key_sounds_and_contours_are_not_retriggered`, `glide_moves_the_keyboard_voltage_between_keys`, `solo`, `oscillators_2_and_3_play_through_their_controls`, `held`, `crossings`, `modulation_and_the_wheels_meet_the_service_manual`, `the_noise_sits_under_the_triangle_as_the_factory_set_it`, `osc3_period`, `oscillator_3s_frequency_and_wide_range_meet_the_service_manual`, `dbu`, `the_a440_and_the_external_input_meet_the_service_manual`.

### crates/model-f-rt/src/lib.rs

Gerçek zamanlı güvenlik, işlem kipi ve bellek/iş parçacığı sınırlarının araçlarını tanımlar.

Fonksiyonlar: `promote`, `demote`, `flush_mode`, `set_flush_mode`, `get`, `set`, `get`, `set`, `get`, `set`, `watch`, `demotions`, `is_realtime`, `tid`, `tid`, `pin`, `pin`, `promote`, `let_go`, `drop`, `register`, `unregister`, `ours`, `start`, `allowed_cpus`, `millis`, `canary`, `watch`, `realtime_threads`, `promote`, `leave`, `drop`, `promote`, `demote`, `promote`, `a_thread_is_promoted`, `new`, `join`, `leave`, `start`, `finish`, `new`, `join`, `leave`, `start`, `finish`, `drop`, `the_flush_bits_round_trip_and_flush`, `the_watchdog_demotes_only_threads_promoted_here`.

### xtask/src/main.rs

Bu modülün main veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.

Fonksiyonlar: `main`.


## C# bilen geliştirici için Rust kodunu okuma

- `fn` metot/fonksiyon, `struct` veri yapısı; `impl Tip` içindeki fonksiyonlar o tipin işlemleridir.
- `&self` nesneyi ödünç okuyabilir; `&mut self` durumu değiştirebilir. `Arc` paylaşılan sahipliktir; `Mutex` bekleme maliyeti taşıyabilir. `Drop` kaynak bırakma aşamasıdır.
- `Option` yok/var, `Result` başarı/hata taşır. `?` hatayı çağırana iletir; `unwrap` beklenmedik hata halinde panic oluşturabilir. Audio callback içine yeni panic yolu eklemeyin.
- `unsafe` C#’taki güvenli tip/ömür denetimlerinden çıkılan bir sınır gibi ele alınır: pointer sahibi, null, tampon boyu ve thread ömrü birlikte incelenir. Mevcut güvenlik yorumunu silmeyin.
- `#[cfg(test)]` yalnız test derlemesinde; `#[test]` test girişidir. `cargo test -p PAKET --release --locked --offline -- --list` test adlarını listeler. Ad filtresiyle tek senaryoyu seçerek başlayabilirsiniz.
- Debug sembolleri için ayrı `cargo build` dev derlemesi kullanın; mevcut release ayarları sembolleri azaltabilir. Rust destekli debugger ile yerel exe/host sürecine bağlanın. `RUST_BACKTRACE=1` bir terminal testindeki panic çağrı yolunu gösterebilir; GUI/host içine dosya logu eklemek yerine testte tekrar üretin.

Türkçe açıklamayı değiştirdiğiniz gerçek tarihte üst bilgiyi de güncelleyin. Önce küçük bir branch/kopya alın; kaynakların yerel taşınması GitHub’a otomatik gönderim değildir.
