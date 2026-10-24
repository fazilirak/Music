// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: F EQ host girişini, parametre/durum kaydını ve DSP callback yaşam döngüsünü bağlar.
mod analog;
pub mod cuts;
pub mod dsp;
mod editor;
pub mod panel;
pub mod params;
mod saturation;
pub mod spectrum;
mod window;
use nih_plug::prelude::*;
use params::EqParams;
use std::sync::{
    Arc,
    atomic::{AtomicU32, Ordering},
};

pub struct SimpleEq {
    params: Arc<EqParams>,
    engine: dsp::Engine,
    meter: Arc<AtomicU32>,
    rate: Arc<AtomicU32>,
    spectrum: Arc<spectrum::Analyzer>,
}
impl Default for SimpleEq {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            params: Arc::new(EqParams::default()),
            engine: dsp::Engine::default(),
            meter: Arc::new(AtomicU32::new(0)),
            rate: Arc::new(AtomicU32::new(48000f32.to_bits())),
            spectrum: Arc::new(spectrum::Analyzer::default()),
        }
    }
}
impl Plugin for SimpleEq {
    // Önceki kayıt biçimindeki alanları güncel host durumuna uyarlayan geçiş sınırıdır.
    // Gövdedeki başlıca çağrılar: entry → into → or_insert → ParamValue::Bool. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn filter_state(state: &mut PluginState) {
        use nih_plug::wrapper::state::ParamValue;
        state
            .params
            .entry("analog".into())
            .or_insert(ParamValue::Bool(false));
        state
            .params
            .entry("saturation".into())
            .or_insert(ParamValue::F32(0.));
        for id in ["low_slope", "high_slope"] {
            state.params.entry(id.into()).or_insert(ParamValue::I32(0));
        }
        // Old sessions keep their existing tilt Q and the former fixed bass Q.
        state
            .params
            .entry("bass_q".into())
            .or_insert(ParamValue::F32(0.70710677));
    }
    const NAME: &'static str = "F EQ";
    const VENDOR: &'static str = "FAZIL IRAK";
    const URL: &'static str = "";
    const EMAIL: &'static str = "";
    const VERSION: &'static str = env!("CARGO_PKG_VERSION");
    const AUDIO_IO_LAYOUTS: &'static [AudioIOLayout] = &[
        AudioIOLayout {
            main_input_channels: Some(new_nonzero_u32(2)),
            main_output_channels: Some(new_nonzero_u32(2)),
            ..AudioIOLayout::const_default()
        },
        AudioIOLayout {
            main_input_channels: Some(new_nonzero_u32(1)),
            main_output_channels: Some(new_nonzero_u32(1)),
            ..AudioIOLayout::const_default()
        },
    ];
    const SAMPLE_ACCURATE_AUTOMATION: bool = true;
    type SysExMessage = ();
    type BackgroundTask = ();
    // Host’un parametre nesnesine/sözleşmesine erişimini sağlar; arayüz metniyle kalıcı kimliği ayırır.
    fn params(&self) -> Arc<dyn Params> {
        self.params.clone()
    }
    // Host’a grafik editör nesnesini sağlar; ses motoru editör kapalıyken de işlemeye devam eder.
    fn editor(&mut self, _: AsyncExecutor<Self>) -> Option<Box<dyn Editor>> {
        Some(Box::new(editor::EqEditor::new(
            self.params.clone(),
            self.meter.clone(),
            self.rate.clone(),
            self.spectrum.clone(),
        )))
    }
    // Host’un örnekleme/kanal yapılandırmasıyla çalışma durumunu başlatır.
    fn initialize(
        &mut self,
        _: &AudioIOLayout,
        c: &BufferConfig,
        _: &mut impl InitContext<Self>,
    ) -> bool {
        self.rate.store(c.sample_rate.to_bits(), Ordering::Relaxed);
        self.engine
            .prepare(c.sample_rate as f64, self.params.settings());
        true
    }
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    fn reset(&mut self) {
        self.engine.prepare(
            f32::from_bits(self.rate.load(Ordering::Relaxed)) as f64,
            self.params.settings(),
        );
        self.meter.store(0, Ordering::Relaxed);
        self.spectrum.reset();
    }
    // İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
    // Gövdedeki başlıca çağrılar: self.engine.set → self.params.settings → b.iter_samples → frame.get_mut. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn process(
        &mut self,
        b: &mut Buffer,
        _: &mut AuxiliaryBuffers,
        _: &mut impl ProcessContext<Self>,
    ) -> ProcessStatus {
        self.engine.set(self.params.settings());
        let mut peak = 0f32;
        for mut frame in b.iter_samples() {
            let left = *frame.get_mut(0).unwrap();
            let right = frame.get_mut(1).map(|s| *s).unwrap_or(left);
            let y = self.engine.sample([left, right]);
            self.spectrum.push(y[0], y[1]);
            *frame.get_mut(0).unwrap() = y[0];
            if let Some(r) = frame.get_mut(1) {
                *r = y[1];
            }
            peak = peak.max(y[0].abs()).max(y[1].abs());
        }
        self.meter.store(peak.to_bits(), Ordering::Relaxed);
        ProcessStatus::Normal
    }
}
impl ClapPlugin for SimpleEq {
    const CLAP_ID: &'static str = "com.fazilirak.simple-eq";
    const CLAP_DESCRIPTION: Option<&'static str> =
        Some("Vintage tilt equalizer with bass lift, low cut and high cut");
    const CLAP_MANUAL_URL: Option<&'static str> = None;
    const CLAP_SUPPORT_URL: Option<&'static str> = None;
    const CLAP_FEATURES: &'static [ClapFeature] = &[
        ClapFeature::AudioEffect,
        ClapFeature::Equalizer,
        ClapFeature::Stereo,
        ClapFeature::Mono,
    ];
}
impl Vst3Plugin for SimpleEq {
    const VST3_CLASS_ID: [u8; 16] = *b"FazilSimpleEQ001";
    const VST3_SUBCATEGORIES: &'static [Vst3SubCategory] =
        &[Vst3SubCategory::Fx, Vst3SubCategory::Eq];
}
nih_export_clap!(SimpleEq);
nih_export_vst3!(SimpleEq);

#[cfg(test)]
mod state_tests {
    use super::*;
    #[test]
    // Regresyon senaryosu legacy_state_explicitly_resets_added_controls: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: into → Default::default → insert → nih_plug::wrapper::state::ParamValue::F32. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn legacy_state_explicitly_resets_added_controls() {
        let mut state = PluginState {
            version: "0.1.0".into(),
            params: Default::default(),
            fields: Default::default(),
        };
        state
            .params
            .insert("q".into(), nih_plug::wrapper::state::ParamValue::F32(1.6));
        SimpleEq::filter_state(&mut state);
        use nih_plug::wrapper::state::ParamValue;
        assert!(matches!(state.params["analog"], ParamValue::Bool(false)));
        assert!(matches!(state.params["saturation"], ParamValue::F32(0.)));
        for id in ["low_slope", "high_slope"] {
            assert!(matches!(state.params[id], ParamValue::I32(0)));
        }
        assert!(matches!(state.params["bass_q"],ParamValue::F32(v) if (v-0.70710677).abs()<1e-7));
        assert!(matches!(state.params["q"], ParamValue::F32(1.6)));
    }
}
