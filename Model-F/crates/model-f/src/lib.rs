// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
//! Model F. GPL-3.0-or-later. Includes circuit DSP copyright 2026 Idle Foundry Ltd.
pub mod arp;
pub mod character;
mod editor;
pub mod engine;
pub mod helper;
pub mod mseg;
mod mseg_ui;
pub mod panel;
pub mod params;
pub mod pool;
pub mod presets;
mod window;

use engine::{Engine, Event};
use helper::{Helper, MEND, SERVE};
use nih_plug::prelude::*;
use params::ModelFParams;
use std::{
    sync::Arc,
    sync::atomic::{AtomicBool, AtomicU32, Ordering},
};

#[derive(Debug)]
pub struct ModelF {
    params: Arc<ModelFParams>,
    engine: Engine,
    arp: arp::Arp,
    helper: Option<Helper>,
    rate: f64,
    meter: Arc<AtomicU32>,
    overload: Arc<AtomicU32>,
    keys: Arc<AtomicU32>,
    panic_requested: Arc<AtomicBool>,
    last_keys: u32,
    playing: bool,
    last_beat: Option<f64>,
    last_mode: i32,
}
impl Default for ModelF {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    // Gövdedeki başlıca çağrılar: Arc::new → ModelFParams::default → Engine::new → arp::Arp::default. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn default() -> Self {
        Self {
            params: Arc::new(ModelFParams::default()),
            engine: Engine::new(),
            arp: arp::Arp::default(),
            helper: None,
            rate: 48000.0,
            meter: Arc::new(AtomicU32::new(0)),
            overload: Arc::new(AtomicU32::new(0)),
            keys: Arc::new(AtomicU32::new(0)),
            panic_requested: Arc::new(AtomicBool::new(false)),
            last_keys: 0,
            playing: false,
            last_beat: None,
            last_mode: 6,
        }
    }
}
impl Drop for ModelF {
    // Kapanışta sahip olunan kaynak ve bağlantıları bırakır; callback’in geçersiz nesneye erişmemesi gerekir.
    fn drop(&mut self) {
        drop(self.helper.take());
    }
}
impl ModelF {
    /// Request the same asynchronous all-sound-off action as the panel button.
    // panic alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
    pub fn panic(&self) {
        self.keys.store(0, Ordering::Release);
        self.panic_requested.store(true, Ordering::Release);
    }
    // parameters alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
    pub fn parameters(&self) -> Arc<ModelFParams> {
        self.params.clone()
    }
    // event alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
    // Gövdedeki başlıca çağrılar: note → engine.event → self.arp.note → Event::PitchBend. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn event(&mut self, e: NoteEvent<()>) {
        let engine = &mut self.engine;
        match e {
            NoteEvent::NoteOn {
                channel,
                note,
                velocity,
                ..
            } => self
                .arp
                .note(channel, note, velocity > 0.0, |e| engine.event(e)),
            NoteEvent::NoteOff { channel, note, .. } | NoteEvent::Choke { channel, note, .. } => {
                self.arp.note(channel, note, false, |e| engine.event(e))
            }
            NoteEvent::MidiPitchBend { value, .. } => {
                engine.event(Event::PitchBend(value * 2.0 - 1.0))
            }
            NoteEvent::MidiCC { cc: 1, value, .. } => engine.event(Event::Modulation(value)),
            NoteEvent::MidiCC { cc: 120, .. } => {
                self.arp.clear(|e| engine.event(e));
                engine.event(Event::AllSoundOff);
            }
            NoteEvent::MidiCC { cc: 123, .. } => self.arp.clear(|e| engine.event(e)),
            NoteEvent::MidiCC { cc: 121, .. } => engine.event(Event::ResetControllers),
            _ => (),
        }
    }
}
impl Plugin for ModelF {
    // Önceki kayıt biçimindeki alanları güncel host durumuna uyarlayan geçiş sınırıdır.
    // Gövdedeki başlıca çağrılar: state.version.as_str → ParamValue::F32 → state.params.get → let. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn filter_state(state: &mut PluginState) {
        if matches!(
            state.version.as_str(),
            "" | "0.1.0" | "0.1.1" | "0.1.2" | "0.1.3"
        ) {
            use nih_plug::wrapper::state::ParamValue;
            if let Some(ParamValue::F32(target)) = state.params.get("mseg_target") {
                let amount = match state.params.get("mseg_amount") {
                    Some(ParamValue::F32(v)) => *v,
                    _ => 0.,
                };
                let (target, amount) = mseg::legacy_target(*target, amount);
                state
                    .params
                    .insert("mseg_target".into(), ParamValue::F32(target));
                state
                    .params
                    .insert("mseg_amount".into(), ParamValue::F32(amount));
            }
        }
        // Old sessions have only the original 27 controls. Reset all new controls
        // when absent, including when loading into an instance with modulation on.
        let defaults = ModelFParams::default();
        for (id, p) in defaults.list() {
            if id.starts_with("mseg_") || id == "overload" || matches!(id, "osc2_waveform" | "osc2_octave" | "osc1_level" | "osc2_level" | "osc1_tune" | "audio_attack" | "audio_decay" | "audio_sustain" | "osc_sync" | "fm_on" | "fm_amount") {
                state
                    .params
                    .entry(id.into())
                    .or_insert(nih_plug::wrapper::state::ParamValue::F32(
                        p.default_plain_value(),
                    ));
            }
        }
    }
    const NAME: &'static str = "Model F";
    const VENDOR: &'static str = "FAZIL IRAK";
    const URL: &'static str = "";
    const EMAIL: &'static str = "";
    const VERSION: &'static str = env!("CARGO_PKG_VERSION");
    const AUDIO_IO_LAYOUTS: &'static [AudioIOLayout] = &[AudioIOLayout {
        main_output_channels: Some(new_nonzero_u32(2)),
        ..AudioIOLayout::const_default()
    }];
    const MIDI_INPUT: MidiConfig = MidiConfig::MidiCCs;
    const SAMPLE_ACCURATE_AUTOMATION: bool = true;
    type SysExMessage = ();
    type BackgroundTask = ();
    // Host’un parametre nesnesine/sözleşmesine erişimini sağlar; arayüz metniyle kalıcı kimliği ayırır.
    fn params(&self) -> Arc<dyn Params> {
        self.params.clone()
    }
    // Host’a grafik editör nesnesini sağlar; ses motoru editör kapalıyken de işlemeye devam eder.
    fn editor(&mut self, _: AsyncExecutor<Self>) -> Option<Box<dyn Editor>> {
        Some(Box::new(editor::ModelFEditor::new(
            self.params.clone(),
            self.meter.clone(),
            self.overload.clone(),
            self.keys.clone(),
            self.panic_requested.clone(),
        )))
    }
    // Host’un örnekleme/kanal yapılandırmasıyla çalışma durumunu başlatır.
    // Gövdedeki başlıca çağrılar: f64::from → self.engine.set → self.params.controls → self.engine.prepare. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn initialize(
        &mut self,
        _: &AudioIOLayout,
        config: &BufferConfig,
        _: &mut impl InitContext<Self>,
    ) -> bool {
        self.rate = f64::from(config.sample_rate);
        self.engine.set(&self.params.controls());
        self.engine.prepare(self.rate, 0x4d6f64656c46);
        if self.helper.is_none() {
            self.helper = Helper::start(self.engine.spares(), self.engine.crew()).ok();
        }
        self.engine.start_workers(
            engine::default_workers(),
            Some(std::time::Duration::from_secs_f64(
                f64::from(config.max_buffer_size.max(1)) / self.rate,
            )),
        );
        true
    }
    // deactivate alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
    fn deactivate(&mut self) {
        self.engine.stop_workers();
    }
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    fn reset(&mut self) {
        self.arp.clear(|e| self.engine.event(e));
        self.engine.release_all();
        self.engine.event(Event::ResetControllers);
        self.playing = false;
        self.last_beat = None;
        self.last_keys = 0;
    }
    // İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
    // Gövdedeki başlıca çağrılar: self.engine.set → self.params.controls → context.transport → self.params.arp_sync.value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn process(
        &mut self,
        buffer: &mut Buffer,
        _: &mut AuxiliaryBuffers,
        context: &mut impl ProcessContext<Self>,
    ) -> ProcessStatus {
        self.engine.set(&self.params.controls());
        let transport = context.transport();
        let sync = self.params.arp_sync.value() > 0.5;
        let beat = transport.pos_beats();
        if sync
            && self.params.arp_on.value() > 0.5
            && ((self.playing && !transport.playing)
                || (transport.playing
                    && beat
                        .zip(self.last_beat)
                        .is_some_and(|(b, old)| b < old - 0.001)))
        {
            self.arp.clear(|e| self.engine.event(e));
        }
        self.playing = transport.playing;
        self.last_beat = beat;
        self.arp
            .configure(&self.params, self.rate, transport.tempo, |e| {
                self.engine.event(e)
            });
        let mode = self.params.mode.value().round() as i32;
        if mode != self.last_mode {
            self.arp.revoice(|e| self.engine.event(e));
            self.last_mode = mode;
        }
        let mut keys = self.keys.load(Ordering::Acquire);
        if self.panic_requested.swap(false, Ordering::AcqRel) {
            self.arp.clear(|e| self.engine.event(e));
            self.engine.event(Event::AllSoundOff);
            self.last_keys = 0;
            keys = 0;
        }
        for n in 0..24 {
            let mask = 1 << n;
            if (keys ^ self.last_keys) & mask != 0 {
                self.arp
                    .note(15, 48 + n as u8, keys & mask != 0, |e| self.engine.event(e));
            }
        }
        self.last_keys = keys & 0x00ffffff;
        let len = buffer.samples();
        let deadline = (context.process_mode() != ProcessMode::Offline).then(|| {
            std::time::Instant::now()
                + std::time::Duration::from_secs_f64(len as f64 / self.rate * engine::DEADLINE)
        });
        self.engine.set_deadline(deadline);
        let out = buffer.as_slice();
        let mut next = context.next_event();
        let mut i = 0;
        while i < len {
            while let Some(e) = next {
                if e.timing() as usize > i {
                    break;
                }
                self.event(e);
                next = context.next_event();
            }
            self.arp.boundary(|e| self.engine.event(e));
            let end = next
                .map_or(len, |e| e.timing() as usize)
                .min(len)
                .min(i + 128)
                .min(i.saturating_add(self.arp.next_in()));
            let n = end - i;
            if let [l, r, ..] = &mut out[..] {
                self.engine.render(&[], &mut l[i..end], &mut r[i..end]);
            }
            self.arp.advance(n);
            i = end;
        }
        while let Some(e) = next {
            self.event(e);
            next = context.next_event();
        }
        let overload = self.engine.end_block(len);
        self.overload.store(overload.to_bits(), Ordering::Relaxed);
        let peak = out
            .iter()
            .flat_map(|c| c.iter())
            .fold(0.0f32, |m, v| m.max(v.abs()));
        self.meter.store(peak.to_bits(), Ordering::Relaxed);
        let asks = if self.engine.take_mending() { MEND } else { 0 }
            | if self.engine.take_serving() { SERVE } else { 0 };
        if let Some(h) = &self.helper {
            h.ask(asks);
        }
        ProcessStatus::KeepAlive
    }
}
impl ClapPlugin for ModelF {
    const CLAP_ID: &'static str = "audio.modelf.synth";
    const CLAP_DESCRIPTION: Option<&'static str> =
        Some("Compact analogue circuit synth with arpeggiator");
    const CLAP_MANUAL_URL: Option<&'static str> = None;
    const CLAP_SUPPORT_URL: Option<&'static str> = None;
    const CLAP_FEATURES: &'static [ClapFeature] = &[
        ClapFeature::Instrument,
        ClapFeature::Synthesizer,
        ClapFeature::Stereo,
    ];
}
impl Vst3Plugin for ModelF {
    const VST3_CLASS_ID: [u8; 16] = *b"ModelFAudioSynth";
    const VST3_SUBCATEGORIES: &'static [Vst3SubCategory] =
        &[Vst3SubCategory::Instrument, Vst3SubCategory::Synth];
}
nih_export_clap!(ModelF);
nih_export_vst3!(ModelF);

#[cfg(test)]
#[allow(unsafe_code)]
mod panic_tests {
    use super::*;
    use nih_plug::context::{PluginApi, process::TestProcessContext};
    struct Init;
    impl InitContext<ModelF> for Init {
        // plugin_api alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
        fn plugin_api(&self) -> PluginApi {
            PluginApi::Standalone
        }
        // execute alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
        fn execute(&self, _: ()) {}
        // set_latency_samples: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
        fn set_latency_samples(&self, _: u32) {}
        // set_current_voice_capacity: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
        fn set_current_voice_capacity(&self, _: u32) {}
    }
    // block alt adımı: Model F host yaşam döngüsü, MIDI/parametre ve Engine render köprüsünü yönetir.
    // Gövdedeki başlıca çağrılar: let → Buffer::default → b.set_slices → s.push. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn block(p: &mut ModelF, c: &mut TestProcessContext<ModelF>) -> f32 {
        let (mut l, mut r) = ([0f32; 256], [0f32; 256]);
        let mut b = Buffer::default();
        // SAFETY: the channel arrays live until the buffer and process call end.
        unsafe {
            b.set_slices(256, |s| {
                s.push(&mut l);
                s.push(&mut r);
            });
        }
        p.process(
            &mut b,
            &mut AuxiliaryBuffers {
                inputs: &mut [],
                outputs: &mut [],
            },
            c,
        );
        b.as_slice_immutable()
            .iter()
            .flat_map(|channel| channel.iter())
            .fold(0f32, |m, x| m.max(x.abs()))
    }
    #[test]
    // Regresyon senaryosu quick_panel_panic_survives_mouse_up_and_silences_every_mode_and_latched_arp: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: ModelF::default → p.parameters → new → params.clone. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn quick_panel_panic_survives_mouse_up_and_silences_every_mode_and_latched_arp() {
        for mode in [6., 0., 1.] {
            for arp in [false, true] {
                let mut p = ModelF::default();
                let params = p.parameters();
                let mut context = TestProcessContext::<ModelF>::new(
                    params.clone() as Arc<dyn Params>,
                    48000.,
                    ProcessMode::Offline,
                );
                for (id, v) in [
                    ("mode", mode),
                    ("decay", 9.),
                    ("sustain", 8.),
                    ("arp_on", if arp { 1. } else { 0. }),
                    ("arp_latch", 1.),
                    ("mseg_enable", 1.),
                    ("mseg_mode", 1.),
                    ("mseg_target", if arp { 3. } else { 1. }),
                    ("mseg_amount", 0.5),
                ] {
                    let param = params.get(id).unwrap();
                    assert!(context.automate(param.as_ptr(), param.preview_normalized(v)));
                }
                assert!(p.initialize(
                    &ModelF::AUDIO_IO_LAYOUTS[0],
                    &BufferConfig {
                        sample_rate: 48000.,
                        min_buffer_size: None,
                        max_buffer_size: 256,
                        process_mode: ProcessMode::Offline
                    },
                    &mut Init
                ));
                if arp {
                    for note in [48, 55, 60] {
                        context.push_event(NoteEvent::NoteOn {
                            timing: 0,
                            voice_id: None,
                            channel: 0,
                            note,
                            velocity: 0.8,
                        });
                    }
                } else {
                    p.keys
                        .store((1 << 0) | (1 << 7) | (1 << 12), Ordering::Release);
                }
                let mut peak = 0f32;
                for _ in 0..60 {
                    peak = peak.max(block(&mut p, &mut context));
                }
                if arp {
                    for note in [48, 55, 60] {
                        context.push_event(NoteEvent::NoteOff {
                            timing: 0,
                            voice_id: None,
                            channel: 0,
                            note,
                            velocity: 0.,
                        });
                    }
                    for _ in 0..30 {
                        peak = peak.max(block(&mut p, &mut context));
                    }
                }
                assert!(
                    peak > 0.001,
                    "must sound before panic: mode {mode}, arp {arp}"
                );
                p.panic();
                // Cursor/button release, keyboard publication and editor close used to
                // overwrite the shared bit before the next audio block could read it.
                for _ in 0..4 {
                    p.keys.store(0, Ordering::Release);
                }
                assert!(p.panic_requested.load(Ordering::Acquire));
                block(&mut p, &mut context);
                assert!(!p.panic_requested.load(Ordering::Acquire));
                for _ in 0..25 {
                    block(&mut p, &mut context);
                }
                for _ in 0..60 {
                    assert!(
                        block(&mut p, &mut context) < 1e-6,
                        "panic did not stay silent: mode {mode}, arp {arp}"
                    );
                }
                context.push_event(NoteEvent::NoteOn {
                    timing: 0,
                    voice_id: None,
                    channel: 0,
                    note: 64,
                    velocity: 0.8,
                });
                let mut peak = 0f32;
                for _ in 0..60 {
                    peak = peak.max(block(&mut p, &mut context));
                }
                assert!(peak > 0.001, "new notes must work after panic");
                p.deactivate();
            }
        }
    }
}

#[cfg(test)]
mod controls_tests;
