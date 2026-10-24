// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
use nih_plug::prelude::*;

#[derive(Params)]
pub struct EqParams {
    #[id = "tilt"]
    pub tilt: FloatParam,
    #[id = "pivot"]
    pub pivot: FloatParam,
    #[id = "bass"]
    pub bass: FloatParam,
    #[id = "q"]
    pub q: FloatParam,
    #[id = "low_cut"]
    pub low_cut: FloatParam,
    #[id = "high_cut"]
    pub high_cut: FloatParam,
    #[id = "volume"]
    pub volume: FloatParam,
    #[id = "bypass"]
    pub bypass: BoolParam,
    #[id = "analog"]
    pub analog: BoolParam,
    #[id = "saturation"]
    pub saturation: FloatParam,
    #[id = "low_slope"]
    pub low_slope: IntParam,
    #[id = "high_slope"]
    pub high_slope: IntParam,
    #[id = "bass_q"]
    pub bass_q: FloatParam,
}
impl Default for EqParams {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    // Gövdedeki başlıca çağrılar: db → FloatParam::new → with_unit → hz. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn default() -> Self {
        // db alt adımı: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
        fn db(name: &str, lo: f32, hi: f32) -> FloatParam {
            FloatParam::new(name, 0., FloatRange::Linear { min: lo, max: hi }).with_unit(" dB")
        }
        // hz alt adımı: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
        fn hz(name: &str, v: f32, lo: f32, hi: f32) -> FloatParam {
            FloatParam::new(
                name,
                v,
                FloatRange::Skewed {
                    min: lo,
                    max: hi,
                    factor: FloatRange::skew_factor(-2.),
                },
            )
            .with_unit(" Hz")
        }
        Self {
            tilt: db("Tilt", -12., 12.),
            pivot: hz("Pivot", 900., 200., 4000.),
            bass: db("Bass Lift", -12., 12.),
            q: FloatParam::new(
                "Tilt Q",
                0.70710677,
                FloatRange::Linear { min: 0.1, max: 2.0 },
            ),
            bass_q: FloatParam::new(
                "Bass Lift Q",
                0.70710677,
                FloatRange::Linear { min: 0.1, max: 2.0 },
            ),
            low_cut: hz("Low Cut (20 = off)", 20., 20., 500.),
            high_cut: hz("High Cut (22000 = off)", 22000., 1000., 22000.),
            volume: db("Volume", -24., 12.),
            bypass: BoolParam::new("Bypass", false).make_bypass(),
            analog: BoolParam::new("Analog", false),
            saturation: FloatParam::new(
                "Saturation",
                0.,
                FloatRange::Linear { min: 0., max: 100. },
            )
            .with_unit(" %"),
            low_slope: IntParam::new("Low Cut Slope", 0, IntRange::Linear { min: 0, max: 4 })
                .with_value_to_string(std::sync::Arc::new(|v| {
                    format!("{} dB/oct", crate::cuts::SLOPES[v as usize])
                })),
            high_slope: IntParam::new("High Cut Slope", 0, IntRange::Linear { min: 0, max: 4 })
                .with_value_to_string(std::sync::Arc::new(|v| {
                    format!("{} dB/oct", crate::cuts::SLOPES[v as usize])
                })),
        }
    }
}
impl EqParams {
    // knobs alt adımı: Host parametre kimliği, aralığı, varsayılanı ve DSP kontrol modeline dönüşüm sözleşmesidir.
    pub fn knobs(&self) -> [&FloatParam; 9] {
        [
            &self.tilt,
            &self.pivot,
            &self.bass,
            &self.bass_q,
            &self.low_cut,
            &self.high_cut,
            &self.volume,
            &self.saturation,
            &self.q,
        ]
    }
    // Host parametrelerinin mevcut değerlerini DSP’nin fiziksel kontrol modeline çevirir.
    // Gövdedeki başlıca çağrılar: self.tilt.value → self.pivot.value → self.bass.value → self.q.value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn settings(&self) -> crate::dsp::Settings {
        crate::dsp::Settings {
            tilt: self.tilt.value() as f64,
            pivot: self.pivot.value() as f64,
            bass: self.bass.value() as f64,
            q: self.q.value() as f64,
            bass_q: self.bass_q.value() as f64,
            low: self.low_cut.value() as f64,
            high: self.high_cut.value() as f64,
            volume: self.volume.value() as f64,
            wet: if self.bypass.value() { 0. } else { 1. },
            analog: if self.analog.value() { 1. } else { 0. },
            saturation: self.saturation.value() as f64 / 100.,
            low_slope: self.low_slope.value() as usize,
            high_slope: self.high_slope.value() as usize,
        }
    }
}
