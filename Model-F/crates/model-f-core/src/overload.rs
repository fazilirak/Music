// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Giriş/mikser aşırı sürüş karakterini hafızalı devre/sayısal modelle uygular.
//! A restrained, short feedback preamp for Model F's Overload control.
//! The return is attenuated before this stage. It rounds the waveform before the
//! ladder rather than driving a delayed high-gain clipper into oscillation.
use crate::preamp::PreampOut;
#[derive(Debug, Clone)]
pub struct Warm {
    input_dc: f64,
    output_dc: f64,
    low: [f64; 2],
    previous: f64,
    mixer_previous: f64,
    mixer_dc: f64,
    mixer_peak: f64,
    gain: f64,
    onset: f64,
    onset_step: f64,
    gate: bool,
    peak: f64,
    dc: f64,
    tone: f64,
    smooth: f64,
    fall: f64,
}
impl Warm {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: exp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn new(rate: f64) -> Self {
        Self {
            input_dc: 0.,
            output_dc: 0.,
            low: [0.; 2],
            previous: 0.08,
            mixer_previous: 0.04,
            mixer_dc: 0.,
            mixer_peak: 0.,
            gain: 0.,
            onset: 0.,
            onset_step: 1. - (-1. / (0.008 * rate)).exp(),
            gate: false,
            peak: 0.,
            dc: 1. - (-std::f64::consts::TAU * 8. / rate).exp(),
            tone: 1. - (-std::f64::consts::TAU * 3600. / rate).exp(),
            smooth: 1. - (-1. / (0.003 * rate)).exp(),
            fall: (-1. / (0.075 * rate)).exp(),
        }
    }
    // gate alt adımı: Giriş/mikser aşırı sürüş karakterini hafızalı devre/sayısal modelle uygular.
    pub fn gate(&mut self, gate: bool) {
        if gate && !self.gate {
            self.onset = 0.;
        }
        self.gate = gate;
    }
    /// Drive the summed oscillators into a rounded mixer stage before the ladder.
    /// This feed-forward stage provides audible saturation without raising loop gain.
    // mixer alt adımı: Giriş/mikser aşırı sürüş karakterini hafızalı devre/sayısal modelle uygular.
    // Gövdedeki başlıca çağrılar: conductance.max → sqrt → max → abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn mixer(&mut self, current: f64, conductance: f64, on: bool) -> (f64, f64) {
        const KNEE: f64 = 0.20;
        const BIAS: f64 = 0.04;
        let input = current / conductance.max(1e-9) / KNEE;
        let amount = self.gain * self.onset;
        let drive = 1. + 12. * amount * amount;
        let x = input * drive + BIAS;
        let y = (x + self.mixer_previous)
            / ((1. + x * x).sqrt() + (1. + self.mixer_previous * self.mixer_previous).sqrt());
        self.mixer_previous = x;
        let y = y - BIAS / (1. + BIAS * BIAS).sqrt();
        self.mixer_dc += self.dc * (y - self.mixer_dc);
        // A fixed ceiling compresses peaks without attenuating the whole patch.
        let wet = (y - self.mixer_dc) * KNEE;
        self.mixer_peak = (self.mixer_peak * self.fall).max((input * drive).abs());
        let light = ((self.mixer_peak - 0.8) / 2.2).clamp(0., 1.);
        let lamp = if on {
            light * light * (3. - 2. * light)
        } else {
            0.
        };
        if !on {
            return (current, 0.);
        }
        (
            (input * KNEE * (1. - amount) + wet * amount) * conductance,
            lamp,
        )
    }
    // İşleme akışını bir örnek/adım ilerletir; hafıza güncelleme sırası duyulan çıktıyı etkiler.
    // Gövdedeki başlıca çağrılar: knob.clamp → sqrt → max → x.abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn tick(&mut self, input: f64, knob: f64, on: bool) -> PreampOut {
        // A broad useful range: gentle thickening first, rounded compression last.
        let target = if on { knob.clamp(0., 1.) } else { 0. };
        self.gain += self.smooth * (target - self.gain);
        self.input_dc += self.dc * (input - self.input_dc);
        // Only the return eases in; the original oscillator/amp attack is untouched.
        self.onset += self.onset_step * (1. - self.onset);
        let x = (input - self.input_dc) * self.gain * self.onset * 6.0;
        // First-order antiderivative antialiasing of x/sqrt(1+x*x).
        // The rationalized divided difference stays accurate at equal inputs.
        // A small fixed bias yields gentle even harmonics; its DC is removed below.
        const BIAS: f64 = 0.08;
        let u = x + BIAS;
        let y = (u + self.previous)
            / ((1. + u * u).sqrt() + (1. + self.previous * self.previous).sqrt());
        self.previous = u;
        let y = y - BIAS / (1. + BIAS * BIAS).sqrt();
        self.output_dc += self.dc * (y - self.output_dc);
        let y = (y - self.output_dc) * 0.8;
        self.low[0] += self.tone * (y - self.low[0]);
        self.low[1] += self.tone * (self.low[0] - self.low[1]);
        // Meter the actual nonlinear-stage drive, with a short analog-style fall.
        self.peak = (self.peak * self.fall).max(x.abs());
        let light = ((self.peak - 0.12) / 0.65).clamp(0., 1.);
        PreampOut {
            amp: self.low[1],
            i_bus: if on { self.low[1] / 33e3 } else { 0. },
            lamp: if on {
                light * light * (3. - 2. * light)
            } else {
                0.
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu silence_and_new_note_return_have_no_startup_impulse: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Warm::new → w.gate → w.tick → o.amp.abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn silence_and_new_note_return_have_no_startup_impulse() {
        for rate in [44100., 48000., 96000.] {
            let mut w = Warm::new(rate);
            w.gate(true);
            for _ in 0..10000 {
                let o = w.tick(0., 1., true);
                assert!(o.amp.abs() < 1e-12);
                assert_eq!(o.lamp, 0.);
            }
            w.gate(false);
            w.gate(true);
            let initial = w.tick(1., 1., true).amp.abs();
            assert!(initial < 0.002, "return must ease in, {rate}: {initial}");
            for n in 0..20000 {
                let x = if n % 7 < 3 { 100. } else { -100. };
                let o = w.tick(x, 1., true);
                assert!(o.amp.is_finite() && o.amp.abs() < 2.);
                assert!((0.0..=1.0).contains(&o.lamp));
            }
        }
    }
}

#[cfg(test)]
mod mixer_tests {
    use super::*;
    // third_harmonic alt adımı: Giriş/mikser aşırı sürüş karakterini hafızalı devre/sayısal modelle uygular.
    // Gövdedeki başlıca çağrılar: Warm::new → w.gate → w.tick → let. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn third_harmonic(knob: f64) -> f64 {
        let mut w = Warm::new(48000.);
        w.gate(true);
        let mut fundamental = [0f64; 2];
        let mut third = [0f64; 2];
        for n in 0..15360 {
            let phase = std::f64::consts::TAU * n as f64 / 192.;
            w.tick(0., knob, true);
            let (y, _) = w.mixer(0.08 * phase.sin() / 10000., 1. / 10000., true);
            if n >= 11520 {
                fundamental[0] += y * phase.cos();
                fundamental[1] += y * phase.sin();
                third[0] += y * (3. * phase).cos();
                third[1] += y * (3. * phase).sin();
            }
        }
        third[0].hypot(third[1]) / fundamental[0].hypot(fundamental[1])
    }
    #[test]
    // Regresyon senaryosu mixer_adds_progressively_audible_harmonics_at_equal_gain: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn mixer_adds_progressively_audible_harmonics_at_equal_gain() {
        let dry = third_harmonic(0.);
        let mid = third_harmonic(0.5);
        let full = third_harmonic(1.);
        println!("third/fundamental: dry {dry}, middle {mid}, full {full}");
        assert!(dry < 1e-8 && mid > 0.01 && full > 0.08 && full > mid * 2.);
    }
}
