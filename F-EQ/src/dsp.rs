// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
use std::f64::consts::PI;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Settings {
    pub tilt: f64,
    pub pivot: f64,
    pub bass: f64,
    pub q: f64,
    pub bass_q: f64,
    pub low: f64,
    pub high: f64,
    pub volume: f64,
    pub wet: f64,
    pub analog: f64,
    pub saturation: f64,
    pub low_slope: usize,
    pub high_slope: usize,
}
impl Default for Settings {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            tilt: 0.,
            pivot: 900.,
            bass: 0.,
            q: 1. / 2f64.sqrt(),
            bass_q: 1. / 2f64.sqrt(),
            low: 20.,
            high: 22000.,
            volume: 0.,
            wet: 1.,
            analog: 0.,
            saturation: 0.,
            low_slope: 0,
            high_slope: 0,
        }
    }
}
#[derive(Clone, Copy, Debug)]
pub struct Coeff {
    b: [f64; 3],
    a: [f64; 2],
}
impl Default for Coeff {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            b: [1., 0., 0.],
            a: [0., 0.],
        }
    }
}
impl Coeff {
    // normalized alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
    fn normalized(b: [f64; 3], a: [f64; 3]) -> Self {
        Self {
            b: b.map(|v| v / a[0]),
            a: [a[1] / a[0], a[2] / a[0]],
        }
    }
    // shelf alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
    // Gövdedeki başlıca çağrılar: powf → hz.min → w.cos → w.sin. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn shelf(rate: f64, hz: f64, db: f64, q: f64, high: bool) -> Self {
        let a = 10f64.powf(db / 40.);
        let w = 2. * PI * hz.min(rate * 0.45) / rate;
        let c = w.cos();
        let t = w.sin() / q * a.sqrt();
        if high {
            Self::normalized(
                [
                    a * ((a + 1.) + (a - 1.) * c + t),
                    -2. * a * ((a - 1.) + (a + 1.) * c),
                    a * ((a + 1.) + (a - 1.) * c - t),
                ],
                [
                    (a + 1.) - (a - 1.) * c + t,
                    2. * ((a - 1.) - (a + 1.) * c),
                    (a + 1.) - (a - 1.) * c - t,
                ],
            )
        } else {
            Self::normalized(
                [
                    a * ((a + 1.) - (a - 1.) * c + t),
                    2. * a * ((a - 1.) - (a + 1.) * c),
                    a * ((a + 1.) - (a - 1.) * c - t),
                ],
                [
                    (a + 1.) + (a - 1.) * c + t,
                    -2. * ((a - 1.) + (a + 1.) * c),
                    (a + 1.) + (a - 1.) * c - t,
                ],
            )
        }
    }
    // cut alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
    pub(crate) fn cut(rate: f64, hz: f64, highpass: bool, q: f64) -> Self {
        let w = 2. * PI * hz.min(rate * 0.45) / rate;
        let c = w.cos();
        let alpha = w.sin() / (2. * q);
        let b = if highpass {
            [(1. + c) / 2., -(1. + c), (1. + c) / 2.]
        } else {
            [(1. - c) / 2., 1. - c, (1. - c) / 2.]
        };
        Self::normalized(b, [1. + alpha, -2. * c, 1. - alpha])
    }
    // magnitude alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
    pub fn magnitude(&self, hz: f64, rate: f64) -> f64 {
        let (r, i) = self.complex(hz, rate);
        (r * r + i * i).sqrt()
    }
    // complex alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
    pub(crate) fn complex(&self, hz: f64, rate: f64) -> (f64, f64) {
        let w = 2. * PI * hz / rate;
        let re = self.b[0] + self.b[1] * w.cos() + self.b[2] * (2. * w).cos();
        let im = -self.b[1] * w.sin() - self.b[2] * (2. * w).sin();
        let ar = 1. + self.a[0] * w.cos() + self.a[1] * (2. * w).cos();
        let ai = -self.a[0] * w.sin() - self.a[1] * (2. * w).sin();
        let den = ar * ar + ai * ai;
        ((re * ar + im * ai) / den, (im * ar - re * ai) / den)
    }
}
// coefficients alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
pub fn coefficients(s: Settings, rate: f64) -> [Coeff; 3] {
    shelf_coefficients(s, rate, [0.; 3])
}
// shelf_coefficients alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
fn shelf_coefficients(s: Settings, rate: f64, offset: [f64; 3]) -> [Coeff; 3] {
    [
        Coeff::shelf(rate, s.pivot, -s.tilt + offset[0] * s.analog, s.q, false),
        Coeff::shelf(rate, s.pivot, s.tilt + offset[1] * s.analog, s.q, true),
        Coeff::shelf(rate, 300., s.bass + offset[2] * s.analog, s.bass_q, false),
    ]
}
// response alt adımı: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir.
// Gövdedeki başlıca çağrılar: coefficients → iter → map → c.magnitude. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn response(s: Settings, hz: f64, rate: f64) -> f64 {
    let gain = coefficients(s, rate)
        .iter()
        .map(|c| c.magnitude(hz, rate))
        .product::<f64>();
    let lo = crate::cuts::magnitude(
        rate,
        s.low,
        hz,
        true,
        s.low_slope,
        ((s.low - 20.) / 1.).clamp(0., 1.),
    );
    let hi = crate::cuts::magnitude(
        rate,
        s.high,
        hz,
        false,
        s.high_slope,
        ((22000. - s.high) / 100.).clamp(0., 1.),
    );
    20. * (gain * lo * hi).max(1e-30).log10() + s.volume
}
#[derive(Default, Debug, Clone, Copy)]
pub(crate) struct Memory {
    z1: f64,
    z2: f64,
}
impl Memory {
    // İşleme akışını bir örnek/adım ilerletir; hafıza güncelleme sırası duyulan çıktıyı etkiler.
    pub(crate) fn tick(&mut self, c: Coeff, x: f64) -> f64 {
        let y = c.b[0] * x + self.z1;
        self.z1 = c.b[1] * x - c.a[0] * y + self.z2;
        self.z2 = c.b[2] * x - c.a[1] * y;
        if self.z1.abs() < 1e-25 {
            self.z1 = 0.;
        }
        if self.z2.abs() < 1e-25 {
            self.z2 = 0.;
        }
        y
    }
}
#[derive(Debug)]
pub struct Engine {
    rate: f64,
    current: Settings,
    target: Settings,
    memory: [[Memory; 3]; 2],
    coeff: [[Coeff; 3]; 2],
    color: crate::analog::Color,
    saturation: [crate::saturation::Saturator; 2],
    low_cut: crate::cuts::Cut,
    high_cut: crate::cuts::Cut,
    remaining: usize,
    phase: usize,
    gain: f64,
}
impl Default for Engine {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    // Gövdedeki başlıca çağrılar: Settings::default → Memory::default → Coeff::default → crate::analog::Color::default. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn default() -> Self {
        Self {
            rate: 48000.,
            current: Settings::default(),
            target: Settings::default(),
            memory: [[Memory::default(); 3]; 2],
            coeff: [[Coeff::default(); 3]; 2],
            color: crate::analog::Color::default(),
            saturation: [crate::saturation::Saturator::default(); 2],
            low_cut: crate::cuts::Cut::default(),
            high_cut: crate::cuts::Cut::default(),
            remaining: 0,
            phase: 0,
            gain: 1.,
        }
    }
}
impl Engine {
    // Örnekleme hızı ve blok kapasitesine bağlı tampon/işleme durumunu ses başlamadan hazırlar.
    pub fn prepare(&mut self, rate: f64, s: Settings) {
        self.rate = rate;
        self.current = s;
        self.target = s;
        self.remaining = 0;
        self.phase = 0;
        self.memory = [[Memory::default(); 3]; 2];
        self.color.reset();
        self.saturation = [crate::saturation::Saturator::default(); 2];
        self.low_cut.reset(s.low_slope);
        self.high_cut.reset(s.high_slope);
        self.rebuild();
    }
    // set: F EQ biquad shelf/kesim katsayıları, durum hafızası, tilt/bass ve bypass işleme zinciridir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    pub fn set(&mut self, s: Settings) {
        if s != self.target {
            self.color.change(
                [-self.target.tilt, self.target.tilt, self.target.bass],
                [-s.tilt, s.tilt, s.bass],
                s.analog > 0.5,
            );
            self.target = s;
            self.remaining = (self.rate * 0.02).max(1.) as usize;
        }
    }
    // Mevcut ayarlara bağlı türetilmiş görünüm/katsayı/cache durumunu yeniden kurar.
    fn rebuild(&mut self) {
        for ch in 0..2 {
            self.coeff[ch] = shelf_coefficients(self.current, self.rate, self.color.offsets[ch]);
        }
        self.low_cut.configure(self.rate, self.current.low, true);
        self.high_cut.configure(self.rate, self.current.high, false);
        self.gain = 10f64.powf(self.current.volume / 20.);
    }
    // Bir ses örneğini filtre/model hafızasından geçirir; bir sonraki örneğe taşınan durum bu adımda güncellenir.
    // Gövdedeki başlıca çağrılar: self.color.smooth → self.low_cut.smooth → self.high_cut.smooth → self.rebuild. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn sample(&mut self, input: [f32; 2]) -> [f32; 2] {
        let moving = self.remaining > 0;
        if self.remaining > 0 {
            let t = 1. / self.remaining as f64;
            macro_rules! step {($($v:ident),*)=>{$(self.current.$v+=(self.target.$v-self.current.$v)*t;)*}}
            step!(
                tilt, pivot, bass, q, bass_q, low, high, volume, wet, analog, saturation
            );
            self.color.smooth(t);
            self.low_cut.smooth(self.target.low_slope, t);
            self.high_cut.smooth(self.target.high_slope, t);
            self.remaining -= 1;
        }
        // Rebuild every eight samples, including the last smoothing sample.
        if moving && (self.phase == 0 || self.remaining == 0) {
            self.rebuild();
        }
        self.phase = (self.phase + 1) % 8;
        let mut out = [0.; 2];
        for ch in 0..2 {
            let dry = input[ch] as f64;
            let mut x = dry;
            for i in 0..3 {
                if self.current.analog > 0. {
                    x += self.color.noise(ch) * self.current.analog;
                }
                x = self.memory[ch][i].tick(self.coeff[ch][i], x);
            }
            if self.current.analog > 0. {
                x += self.color.noise(ch) * self.current.analog;
            }
            x = self
                .low_cut
                .sample(ch, x, ((self.current.low - 20.) / 1.).clamp(0., 1.));
            x = self
                .high_cut
                .sample(ch, x, ((22000. - self.current.high) / 100.).clamp(0., 1.));
            x = self.saturation[ch].sample(x, self.current.saturation);
            out[ch] = (dry + (x * self.gain - dry) * self.current.wet) as f32;
        }
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu neutral_is_transparent: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn neutral_is_transparent() {
        let mut e = Engine::default();
        for n in 0..10000 {
            let x = (n as f32 * 0.13).sin() * 0.4;
            assert!((e.sample([x, -x])[0] - x).abs() < 1e-6);
        }
    }
    #[test]
    // Regresyon senaryosu tilt_has_opposite_shelves_and_fixed_pivot: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn tilt_has_opposite_shelves_and_fixed_pivot() {
        let s = Settings {
            tilt: 6.,
            ..Settings::default()
        };
        assert!(response(s, 40., 48000.) < -5.5);
        assert!(response(s, 15000., 48000.) > 5.5);
        assert!(response(s, 900., 48000.).abs() < 1e-7);
    }
    #[test]
    // Regresyon senaryosu bass_and_cuts: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Settings::default → response → abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn bass_and_cuts() {
        let s = Settings {
            bass: 6.,
            ..Settings::default()
        };
        assert!(response(s, 30., 48000.) > 5.9);
        assert!(response(s, 6000., 48000.).abs() < 0.01);
        let s = Settings {
            low: 100.,
            high: 5000.,
            ..Settings::default()
        };
        assert!(response(s, 25., 48000.) < -23.);
        assert!(response(s, 15000., 48000.) < -24.);
        assert!(response(s, 1000., 48000.).abs() < 0.1);
    }
    #[test]
    // Regresyon senaryosu extremes_stable_and_stereo_independent: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Engine::default → e.prepare → Settings::default → e.set. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn extremes_stable_and_stereo_independent() {
        for rate in [22050., 44100., 48000., 96000., 192000.] {
            let mut e = Engine::default();
            e.prepare(rate, Settings::default());
            for n in 0..30000 {
                if n % 1000 == 0 {
                    e.set(Settings {
                        tilt: if n % 2000 == 0 { 12. } else { -12. },
                        bass: 12.,
                        q: 2.,
                        bass_q: if n % 2000 == 0 { 0.1 } else { 2. },
                        pivot: 4000.,
                        low: 500.,
                        high: 1000.,
                        volume: 12.,
                        wet: 1.,
                        low_slope: (n / 1000) % 5,
                        high_slope: (n / 1000) % 5,
                        ..Settings::default()
                    });
                }
                let y = e.sample([if n % 511 == 0 { 0.5 } else { 0. }, 0.]);
                assert!(y[0].is_finite() && y[0].abs() < 100.);
                assert_eq!(y[1], 0.);
            }
        }
    }
    #[test]
    // Regresyon senaryosu bypass_becomes_exact_dry: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn bypass_becomes_exact_dry() {
        let mut e = Engine::default();
        e.set(Settings {
            tilt: 10.,
            wet: 0.,
            ..Settings::default()
        });
        for _ in 0..2000 {
            e.sample([0.2, -0.4]);
        }
        assert_eq!(e.sample([0.123, -0.27]), [0.123, -0.27]);
    }
    #[test]
    // Regresyon senaryosu tilt_and_bass_q_control_separate_shelves: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Settings::default → response → abs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn tilt_and_bass_q_control_separate_shelves() {
        let bass = Settings {
            bass: 6.,
            bass_q: 0.3,
            ..Settings::default()
        };
        assert!(
            (response(bass, 450., 48000.)
                - response(
                    Settings {
                        bass_q: 1.8,
                        ..bass
                    },
                    450.,
                    48000.
                ))
            .abs()
                > 0.5
        );
        for f in [40., 150., 450., 900., 4000.] {
            assert!(
                (response(bass, f, 48000.) - response(Settings { q: 1.8, ..bass }, f, 48000.))
                    .abs()
                    < 1e-7,
                "Tilt Q changed a bass-only curve"
            );
            let tilt = Settings {
                tilt: 6.,
                q: 0.3,
                ..Settings::default()
            };
            assert!(
                (response(tilt, f, 48000.)
                    - response(
                        Settings {
                            bass_q: 1.8,
                            ..tilt
                        },
                        f,
                        48000.
                    ))
                .abs()
                    < 1e-7,
                "Bass Q changed a tilt-only curve"
            );
        }
    }
    #[test]
    // Regresyon senaryosu analog_noise_is_subtle_and_bypass_removes_it_and_saturation: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Engine::default → e.prepare → Settings::default → e.sample. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn analog_noise_is_subtle_and_bypass_removes_it_and_saturation() {
        let mut e = Engine::default();
        e.prepare(
            48000.,
            Settings {
                analog: 1.,
                saturation: 0.7,
                ..Settings::default()
            },
        );
        let mut energy = 0.;
        let mut cross = 0.;
        let mut r_energy = 0.;
        for _ in 0..48000 {
            let y = e.sample([0.; 2]);
            energy += (y[0] as f64).powi(2);
            r_energy += (y[1] as f64).powi(2);
            cross += y[0] as f64 * y[1] as f64;
        }
        let rms = (energy / 48000.).sqrt();
        assert!(rms > 1e-6 && rms < 2e-5);
        assert!((cross / (energy * r_energy).sqrt()).abs() < 0.05);
        e.set(Settings {
            analog: 1.,
            saturation: 1.,
            wet: 0.,
            ..Settings::default()
        });
        for _ in 0..2000 {
            e.sample([0.; 2]);
        }
        assert_eq!(e.sample([0.; 2]), [0.; 2]);
        assert_eq!(e.sample([0.2, -0.3]), [0.2, -0.3]);
    }
    #[test]
    // Regresyon senaryosu actual_audio_measures_96_db_per_octave_and_click_free_switching: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Engine::default → Settings::default → e.prepare → sin. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn actual_audio_measures_96_db_per_octave_and_click_free_switching() {
        let mut e = Engine::default();
        let s = Settings {
            high: 2000.,
            high_slope: 4,
            ..Settings::default()
        };
        e.prepare(192000., s);
        let mut sum = 0.;
        let mut reference = 0.;
        for i in 0..192000 {
            let x = (2. * PI * 8000. * i as f64 / 192000.).sin() * 0.2;
            let y = e.sample([x as f32, 0.])[0] as f64;
            if i > 96000 {
                sum += y * y;
                reference += x * x;
            }
        }
        assert!(10. * (sum / reference).log10() < -180.);
        let mut last = e.sample([0.1; 2])[0];
        for i in 0..10000 {
            if i % 1000 == 0 {
                e.set(Settings {
                    low: 80.,
                    high: 2000.,
                    low_slope: (i / 1000) % 5,
                    high_slope: 4 - (i / 1000) % 5,
                    ..Settings::default()
                });
            }
            let y = e.sample([0.1; 2])[0];
            assert!(y.is_finite());
            assert!((y - last).abs() < 0.05);
            last = y;
        }
    }
}
