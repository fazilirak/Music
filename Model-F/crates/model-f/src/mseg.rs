// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
//! Breakpoint modulation adapted from FAZIL IRAK's Machine envelope behavior.
//! Fixed parameter slots and storage; no allocation, locks or file I/O in audio.
use crate::params::ModelFParams;
use nih_plug::prelude::*;
use serde::{Deserialize, Serialize};


// Fazıl Irak, 24.10.2026: çarpan bir tam zarf döngüsünün hızıdır; x1=8 dörtlük vuruş.
// Free eski presetlerdeki saniye değerini aynen kullanır. Fix Tempo her zaman 120 BPM'dir.
pub const SPEEDS: [&str; 7] = ["FREE", "x1", "x2", "x4", "x8", "x16", "x32"];
pub fn tempo_bpm(p: &ModelFParams, host: Option<f64>) -> f64 {
    if p.mseg_fix_tempo.value() > 0.5 { return 120.0; }
    host.filter(|v| v.is_finite() && *v > 0.0)
        .unwrap_or(f64::from(p.tempo.value())).clamp(20.0,400.0)
}
pub fn duration_seconds(p: &ModelFParams, host: Option<f64>) -> f64 {
    let speed=p.mseg_speed.value().round().clamp(0.0,6.0) as u32;
    if speed==0 { return f64::from(p.mseg_duration.value()); }
    60.0/tempo_bpm(p,host) * 8.0/f64::from(2_u32.pow(speed-1))
}

pub const CAP: usize = 16;
pub const WAVES: [&str; 9] = [
    "LINEAR", "SINE", "TRIANGLE", "SAW UP", "SAW DOWN", "SQUARE", "PULSE", "STEPS", "RANDOM",
];
pub const TARGETS: [&str; 3] = ["FILTER", "OSCILLATOR PITCH", "VOLUME (OUTPUT)"];

/// Convert the six destinations used by versions through 0.1.3.
// legacy_target alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
pub fn legacy_target(target: f32, amount: f32) -> (f32, f32) {
    match target as i32 {
        0 => (1., 0.),
        1 => (1., amount),
        2 | 3 => (2., amount),
        4 | 5 => (3., amount.abs()),
        _ => (1., 0.),
    }
}
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Point {
    pub x: f64,
    pub y: f64,
    pub on: bool,
}
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Bend {
    pub incoming: f64,
    pub outgoing: f64,
    pub wave: usize,
    pub depth: f64,
}
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Config {
    pub enabled: bool,
    pub duration: f64,
    pub curve: f64,
    pub mode: usize,
    pub attack: f64,
    pub decay: f64,
    pub amount: f64,
    pub target: usize,
    pub start: f64,
    pub points: [Point; CAP],
    pub bends: [Bend; CAP + 2],
}
impl Default for Config {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    // Gövdedeki başlıca çağrılar: std::array::from_fn → get → copied → unwrap_or. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn default() -> Self {
        Self {
            enabled: false,
            duration: 1.,
            curve: 0.,
            mode: 0,
            attack: 0.,
            decay: 0.,
            amount: 0.,
            target: 1,
            start: 0.,
            points: std::array::from_fn(|i| Point {
                x: [0.12, 0.28, 0.45, 0.62, 0.78, 0.9]
                    .get(i)
                    .copied()
                    .unwrap_or((i + 1) as f64 / 18.),
                y: [1., 0.2, 0.75, 0.15, 0.5, 0.3]
                    .get(i)
                    .copied()
                    .unwrap_or(0.),
                on: i < 6,
            }),
            bends: [Bend {
                incoming: 0.,
                outgoing: 0.,
                wave: 0,
                depth: 1.,
            }; CAP + 2],
        }
    }
}
impl Config {
    // from_params alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
    // Gövdedeki başlıca çağrılar: f64::from → p.get → unwrap → value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn from_params(p: &ModelFParams) -> Self {
        // Indexed references are generated once in the parameter implementation.
        let get = |id: &str| f64::from(p.get(id).unwrap().value());
        let (points, bends) = p.envelope_slots();
        Self {
            enabled: get("mseg_enable") > 0.5,
            duration: duration_seconds(p,None),
            curve: get("mseg_curve"),
            mode: get("mseg_mode") as usize,
            attack: get("mseg_attack"),
            decay: get("mseg_decay"),
            amount: get("mseg_amount"),
            target: get("mseg_target") as usize,
            start: get("mseg_start"),
            points,
            bends,
        }
    }
    // shape alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
    pub fn shape(&self) -> Shape {
        Shape::new(self)
    }
}
#[derive(Debug, Clone, Copy, Default)]
pub struct Node {
    pub x: f64,
    pub y: f64,
    pub id: usize,
    pub bend: Bend,
}
impl Default for Bend {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            incoming: 0.,
            outgoing: 0.,
            wave: 0,
            depth: 1.,
        }
    }
}
#[derive(Debug, Clone)]
pub struct Shape {
    pub nodes: [Node; CAP + 2],
    pub count: usize,
    pub curve: f64,
}
// fraction alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
pub fn fraction(b: f64, incoming: bool) -> f64 {
    let centre = if incoming { 2. / 3. } else { 1. / 3. };
    centre + b * if b >= 0. { 1. - centre } else { centre }
}
// bend_from_fraction alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
pub fn bend_from_fraction(v: f64, incoming: bool) -> f64 {
    let c = if incoming { 2. / 3. } else { 1. / 3. };
    ((v - c) / if v >= c { 1. - c } else { c }).clamp(-1., 1.)
}
// wave alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
// Gövdedeki başlıca çağrılar: libm::cos → abs → floor → min. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn wave(kind: usize, t: f64) -> f64 {
    if t <= 0. || t >= 1. {
        return 0.;
    }
    match kind {
        1 => 0.5 - 0.5 * libm::cos(std::f64::consts::TAU * t),
        2 => 1. - (2. * t - 1.).abs(),
        3 => t,
        4 => 1. - t,
        5 => {
            if t < 0.5 {
                1.
            } else {
                0.
            }
        }
        6 => {
            if t < 0.25 {
                1.
            } else {
                0.
            }
        }
        7 => (t * 4.).floor() / 3.,
        8 => [0.25, 0.8, 0.45, 1., 0.1, 0.65, 0.35, 0.9][((t * 8.) as usize).min(7)],
        _ => 0.,
    }
}
impl Shape {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: Node::default → c.points.iter → enumerate → p.x.clamp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn new(c: &Config) -> Self {
        let mut s = Self {
            nodes: [Node::default(); CAP + 2],
            count: 1,
            curve: c.curve,
        };
        s.nodes[0] = Node {
            x: 0.,
            y: c.start,
            id: 0,
            bend: c.bends[0],
        };
        for (i, p) in c.points.iter().enumerate() {
            if p.on {
                s.nodes[s.count] = Node {
                    x: p.x.clamp(0.001, 0.999),
                    y: p.y,
                    id: i + 1,
                    bend: c.bends[i + 1],
                };
                s.count += 1;
            }
        }
        // Stable insertion sort avoids allocator-backed sorting paths.
        for i in 2..s.count {
            let n = s.nodes[i];
            let mut j = i;
            while j > 1 && s.nodes[j - 1].x > n.x {
                s.nodes[j] = s.nodes[j - 1];
                j -= 1;
            }
            s.nodes[j] = n;
        }
        for i in 1..s.count {
            s.nodes[i].x = (s.nodes[i].x.min(1. - (s.count - i) as f64 * 0.00001))
                .max(s.nodes[i - 1].x + 0.00001);
        }
        s.nodes[s.count] = Node {
            x: 1.,
            y: c.start,
            id: CAP + 1,
            bend: c.bends[CAP + 1],
        };
        s.count += 1;
        s
    }
    // value alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
    // Gövdedeki başlıca çağrılar: windows → let → clamp → fraction. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn value(&self, x: f64) -> f64 {
        for pair in self.nodes[..self.count].windows(2) {
            let (a, b) = (pair[0], pair[1]);
            if x <= b.x {
                let t = ((x - a.x) / (b.x - a.x)).clamp(0., 1.);
                let u = 1. - t;
                let q = (3. * u * u * t * fraction(a.bend.outgoing, false)
                    + 3. * u * t * t * fraction(b.bend.incoming, true)
                    + t * t * t
                    + self.curve * t * u)
                    .clamp(0., 1.);
                return (a.y
                    + (b.y - a.y) * q
                    + if x < b.x {
                        a.bend.depth * wave(a.bend.wave, q)
                    } else {
                        0.
                    })
                .clamp(0., 1.);
            }
        }
        self.nodes[self.count - 1].y
    }
}
#[derive(Debug, Clone)]
pub struct Envelope {
    config: Config,
    shape: Shape,
    phase: f64,
    gate: bool,
    started: bool,
    output: f64,
    release: Option<(f64, f64)>,
    edit: f64,
    edit_start: f64,
    ad: f64,
    ad_stage: u8,
    ad_coeff: [f64; 2],
    rate: f64,
}
impl Default for Envelope {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        let c = Config::default();
        Self {
            shape: c.shape(),
            config: c,
            phase: 0.,
            gate: false,
            started: false,
            output: 0.,
            release: None,
            edit: 1.,
            edit_start: 0.,
            ad: 0.,
            ad_stage: 0,
            ad_coeff: [0.; 2],
            rate: 0.,
        }
    }
}
impl Envelope {
    // configure: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    // Gövdedeki başlıca çağrılar: self.reset → c.shape → powi. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn configure(&mut self, c: Config, rate: f64) {
        if self.config != c || self.rate != rate {
            // Yalnız tempo değiştiyse fazı ve sesi kesmeden yeni örnek adımına geç.
            let mut timing_only=self.config;timing_only.duration=c.duration;
            if timing_only==c && self.rate==rate { self.config=c;return; }
            if self.config.enabled != c.enabled {
                self.reset();
            }
            if self.started {
                self.edit = 0.;
                self.edit_start = self.output;
            }
            self.shape = c.shape();
            self.config = c;
            self.rate = rate;
            let f = 0.004 * 44100. / rate;
            self.ad_coeff = [
                0.0003 * f + f * (1. - c.attack * 0.5).powi(24) * 7.,
                0.0003 * f + f * (1. - c.decay * 0.5).powi(23) * 7.,
            ];
        }
    }
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    pub fn reset(&mut self) {
        self.phase = 0.;
        self.gate = false;
        self.started = false;
        self.output = 0.;
        self.release = None;
        self.ad = 0.;
        self.ad_stage = 0;
        self.edit = 1.;
    }
    // İşleme akışını bir örnek/adım ilerletir; hafıza güncelleme sırası duyulan çıktıyı etkiler.
    // Gövdedeki başlıca çağrılar: min → self.shape.value → self.phase.floor → self.phase.min. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn tick(&mut self, gate: bool) -> f64 {
        let c = self.config;
        if gate && !self.gate {
            self.phase = 0.;
            self.started = true;
            self.release = None;
            self.ad = 0.;
            self.ad_stage = 0;
        }
        if !gate && self.gate && (!c.enabled || c.mode != 0) {
            self.release = Some((self.output, 0.));
        }
        self.gate = gate;
        if !self.started {
            return 0.;
        }
        let mut y = if let Some((start, pos)) = self.release {
            let pos = (pos + 1. / (self.rate * 0.02)).min(1.);
            self.release = Some((start, pos));
            start * (1. - pos)
        } else if c.enabled {
            let y = self.shape.value(self.phase);
            self.phase += 1. / (self.rate * c.duration);
            if c.mode == 1 && gate && self.phase >= 1. {
                self.phase -= self.phase.floor();
            }
            if c.mode == 2 && gate {
                self.phase = self.phase.min(self.shape.nodes[self.shape.count - 2].x);
            }
            self.phase = self.phase.min(1.);
            y
        } else {
            if self.ad_stage == 0 {
                self.ad =
                    (self.ad + self.ad_coeff[0] * (self.ad + 0.004 * 44100. / self.rate)).min(1.);
                if self.ad >= 1. {
                    self.ad_stage = 1;
                }
            } else {
                self.ad =
                    (self.ad - self.ad_coeff[1] * (self.ad + 0.004 * 44100. / self.rate)).max(0.);
            }
            if self.ad_stage == 0 && c.attack == 0. {
                1.
            } else {
                self.ad
            }
        };
        if self.edit < 1. {
            self.edit = (self.edit + 1. / (self.rate * 0.001)).min(1.);
            y = self.edit_start + (y - self.edit_start) * self.edit;
        }
        self.output = y.clamp(0., 1.);
        self.output
    }
    // route alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
    // Gövdedeki başlıca çağrılar: self.tick → envelope → self.config.amount.abs → min. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn route(&mut self, gate: bool, j: &mut model_f_core::voice::Jacks) -> f64 {
        // Muting depth must not freeze time or lose note-off.
        let value = self.tick(gate);
        if self.config.amount == 0. {
            return 1.;
        }
        let y = value * self.config.amount;
        match self.config.target {
            1 => j.cutoff += y * 4.,
            2 => {
                j.detune[0] += y * 4800.;
                j.detune[1] += y * 4800.;
            }
            3 => {
                // Crossfade dry output to the envelope (negative depth inverts it).
                // Unity at zero depth; bounded 0..1 so modulation cannot overload output.
                let depth = self.config.amount.abs().min(1.);
                let shape = if self.config.amount >= 0. {
                    value
                } else {
                    1. - value
                };
                return 1. - depth + depth * shape;
            }
            _ => (),
        }
        1.
    }
}

// saved_id: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
fn saved_id(id: &str) -> bool {
    id.starts_with("mseg_")
        && !matches!(
            id,
            "mseg_attack" | "mseg_decay" | "mseg_amount" | "mseg_target"
        )
}
#[derive(Debug, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct EnvelopePreset {
    pub format: u32,
    pub values: std::collections::BTreeMap<String, f32>,
}
impl EnvelopePreset {
    // capture: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    pub fn capture(p: &ModelFParams) -> Self {
        Self {
            format: 1,
            values: p
                .list()
                .into_iter()
                .filter(|(id, _)| saved_id(id))
                .map(|(id, p)| (id.into(), p.value()))
                .collect(),
        }
    }
    // validate alt adımı: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar.
    // Gövdedeki başlıca çağrılar: ModelFParams::default → Self::capture → self.values.len → expected.values.len. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn validate(&self) -> Result<(), String> {
        let p = ModelFParams::default();
        let expected = Self::capture(&p);
        if self.format != 1 || ![expected.values.len(),expected.values.len()-2].contains(&self.values.len()) || expected.values.keys().any(|id| !self.values.contains_key(id) && !matches!(id.as_str(),"mseg_speed" | "mseg_fix_tempo")) {
            return Err("Unsupported or incomplete envelope preset".into());
        }
        for (id, v) in &self.values {
            if !expected.values.contains_key(id) {
                return Err(format!("Unknown envelope control: {id}"));
            }
            let param = p.get(id).unwrap();
            if !v.is_finite()
                || *v < param.preview_plain(0.)
                || *v > param.preview_plain(1.)
                || (*v - param.preview_plain(param.preview_normalized(*v))).abs() > 0.0002
            {
                return Err(format!("Invalid envelope value: {id}"));
            }
        }
        Ok(())
    }
    // apply: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    pub fn apply(&self, p: &ModelFParams, setter: &ParamSetter<'_>) -> Result<(), String> {
        self.validate()?;
        for (id,param) in p.list().into_iter().filter(|(id,_)| saved_id(id)) {
            setter.begin_set_parameter(param);
            setter.set_parameter(param,self.values.get(id).copied().unwrap_or_else(||param.default_plain_value()));
            setter.end_set_parameter(param);
        }
        Ok(())
    }
    // save: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    pub fn save(&self, path: &std::path::Path) -> Result<(), String> {
        self.validate()?;
        crate::presets::atomic_write(
            path,
            &serde_json::to_vec_pretty(self).map_err(|e| e.to_string())?,
        )
        .map_err(|e| e.to_string())
    }
    // read: Nokta/eğri/wave brush envelope’unun fiziksel zaman ve hedef modülasyon değerini hesaplar. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    pub fn read(path: &std::path::Path) -> Result<Self, String> {
        if std::fs::metadata(path).map_err(|e| e.to_string())?.len() > 128 * 1024 {
            return Err("Envelope file is too large".into());
        }
        let p: Self = serde_json::from_slice(&std::fs::read(path).map_err(|e| e.to_string())?)
            .map_err(|e| e.to_string())?;
        p.validate()?;
        Ok(p)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu machine_shape_timing_playback_and_release: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Config::default → Envelope::default → env.configure → env.tick. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn machine_shape_timing_playback_and_release() {
        for rate in [44100., 48000., 96000.] {
            let mut c = Config {
                enabled: true,
                duration: 0.5,
                ..Config::default()
            };
            let mut env = Envelope::default();
            env.configure(c, rate);
            for _ in 0..(rate * 0.03) as usize {
                env.tick(true);
            }
            assert!((env.tick(true) - 0.5).abs() < 0.001);
            for _ in 0..rate as usize {
                env.tick(true);
            }
            assert_eq!(env.output, 0.);
            c.mode = 1;
            env.configure(c, rate);
            env.reset();
            for _ in 0..(rate * 0.03) as usize {
                env.tick(true);
            }
            let first = env.tick(true);
            for _ in 0..(rate * 0.5) as usize - 1 {
                env.tick(true);
            }
            assert!((env.tick(true) - first).abs() < 0.001);
            c.mode = 2;
            env.configure(c, rate);
            env.reset();
            for _ in 0..rate as usize {
                env.tick(true);
            }
            assert!((env.output - 0.3).abs() < 0.001);
            let held = env.output;
            assert!((held - env.tick(false)).abs() < 0.001);
            for _ in 0..(rate * 0.021) as usize {
                env.tick(false);
            }
            assert_eq!(env.output, 0.);
        }
    }
    #[test]
    // Regresyon senaryosu all_waves_endpoints_curves_and_crossed_slots_stay_bounded: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Config::default → c.shape → shape.value → contains. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn all_waves_endpoints_curves_and_crossed_slots_stay_bounded() {
        for kind in 1..=8 {
            let mut c = Config::default();
            for p in &mut c.points {
                p.on = false;
            }
            c.bends[0].wave = kind;
            c.bends[0].depth = 0.75;
            let shape = c.shape();
            assert_eq!(shape.value(0.), 0.);
            assert_eq!(shape.value(1.), 0.);
            let mut peak = 0f64;
            for i in 0..=1000 {
                let y = shape.value(i as f64 / 1000.);
                assert!((0.0..=0.750001).contains(&y));
                peak = peak.max(y);
            }
            assert!(peak > 0.7);
        }
        let mut c = Config::default();
        c.start = 0.65;
        for (i, p) in c.points.iter_mut().enumerate() {
            p.on = true;
            p.x = if i % 2 == 0 { 0. } else { 1. };
        }
        let shape = c.shape();
        assert_eq!(shape.count, 18);
        assert!(
            shape.nodes[..shape.count]
                .windows(2)
                .all(|n| n[0].x < n[1].x)
        );
        assert_eq!(shape.value(0.), 0.65);
        assert_eq!(shape.value(1.), 0.65);
        for i in 0..1000 {
            assert!(shape.value(i as f64 / 1000.).is_finite());
        }
        c = Config::default();
        let before = c.shape();
        c.bends[1].incoming = 1.;
        let after = c.shape();
        assert!((before.value(0.06) - after.value(0.06)).abs() > 0.01);
        assert_eq!(before.value(0.2), after.value(0.2));
    }
    #[test]
    // Regresyon senaryosu routing_and_envelope_files_do_not_change_other_synth_controls: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Envelope::default → Config::default → env.configure → env.reset. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn routing_and_envelope_files_do_not_change_other_synth_controls() {
        let mut env = Envelope::default();
        let mut c = Config {
            enabled: true,
            start: 0.5,
            amount: 0.5,
            ..Config::default()
        };
        for target in 1..=3 {
            c.target = target;
            env.configure(c, 48000.);
            env.reset();
            let mut j = model_f_core::voice::Jacks::default();
            let gain = env.route(true, &mut j);
            match target {
                1 => assert!(j.cutoff > 0.),
                2 => assert!(j.detune[0] > 0. && j.detune[1] == j.detune[0]),
                3 => assert!(gain > 0. && gain < 1.),
                _ => unreachable!(),
            }
        }
        let p = ModelFParams::default();
        let preset = EnvelopePreset::capture(&p);
        assert!(!preset.values.contains_key("mseg_target"));
        assert!(!preset.values.contains_key("mseg_amount"));
        assert!(!preset.values.contains_key("cutoff"));
        let temp = tempfile::tempdir().unwrap();
        let path = temp.path().join("shape.modelfenv");
        preset.save(&path).unwrap();
        assert_eq!(EnvelopePreset::read(&path).unwrap().values, preset.values);
        let mut bad = preset;
        bad.values.insert("mseg_p0_x".into(), 2.);
        assert!(bad.validate().is_err());
    }
    #[test]
    // Regresyon senaryosu zero_depth_keeps_note_timing: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Default::default → Envelope::default → e.configure → e.route. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn zero_depth_keeps_note_timing() {
        let mut c = Config {
            enabled: true,
            duration: 0.5,
            target: 1,
            ..Default::default()
        };
        let mut e = Envelope::default();
        e.configure(c, 48000.);
        for _ in 0..1440 {
            e.route(true, &mut Default::default());
        }
        c.amount = 0.5;
        e.configure(c, 48000.);
        let mut j = model_f_core::voice::Jacks::default();
        e.route(true, &mut j);
        assert!(j.cutoff > 0.9, "resuming depth must not restart at zero");
        c.amount = 0.;
        c.mode = 2;
        e.configure(c, 48000.);
        e.route(false, &mut Default::default());
        for _ in 0..1000 {
            e.route(false, &mut Default::default());
        }
        assert_eq!(e.output, 0.);
    }
    #[test]
    // Regresyon senaryosu output_gain_depth_and_polarity_are_bounded: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Default::default → Envelope::default → e.configure → e.route. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn output_gain_depth_and_polarity_are_bounded() {
        for amount in [-1., -0.5, 0., 0.5, 1.] {
            let c = Config {
                enabled: true,
                target: 3,
                amount,
                duration: 0.05,
                ..Default::default()
            };
            let mut e = Envelope::default();
            e.configure(c, 48000.);
            for _ in 0..3000 {
                let mut j = Default::default();
                let gain = e.route(true, &mut j);
                assert!((0.0..=1.0).contains(&gain));
                if amount == 0. {
                    assert_eq!(gain, 1.);
                }
                assert_eq!(j, model_f_core::voice::Jacks::default());
            }
        }
        assert_eq!(TARGETS, ["FILTER", "OSCILLATOR PITCH", "VOLUME (OUTPUT)"]);
    }
}

#[cfg(test)]
mod tempo_tests {
    use super::*;
    use std::sync::Arc;
    use nih_plug::context::process::TestProcessContext;
    #[test]
    fn speed_uses_host_bpm_fixed_120_or_legacy_seconds() {
        let p=Arc::new(ModelFParams::default());
        let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.0,ProcessMode::Offline);
        let set=|id:&str,v:f32| {let q=p.get(id).unwrap();assert!(ctx.automate(q.as_ptr(),q.preview_normalized(v)));};
        set("mseg_duration",7.25);
        for bpm in [Some(60.0),Some(180.0),None] {assert_eq!(duration_seconds(&p,bpm),f64::from(p.mseg_duration.value()));}
        for speed in 1..=6 {
            set("mseg_speed",speed as f32);
            let beats=8.0/2_f64.powi(speed-1);
            assert!((duration_seconds(&p,Some(60.0))-beats).abs()<1e-12);
            assert!((duration_seconds(&p,Some(120.0))-beats*0.5).abs()<1e-12);
            set("mseg_fix_tempo",1.0);
            assert_eq!(tempo_bpm(&p,Some(210.0)),120.0);
            assert!((duration_seconds(&p,Some(60.0))-beats*0.5).abs()<1e-12);
            set("mseg_fix_tempo",0.0);
        }
        set("tempo",90.0);
        assert_eq!(tempo_bpm(&p,None),f64::from(p.tempo.value()));
        for bad in [f64::NAN,f64::INFINITY,0.0,-10.0] {assert_eq!(tempo_bpm(&p,Some(bad)),f64::from(p.tempo.value()));}
        set("mseg_speed",0.0);set("mseg_fix_tempo",1.0);assert_eq!(duration_seconds(&p,Some(60.0)),f64::from(p.mseg_duration.value()));
    }
    #[test]
    fn changing_only_tempo_preserves_phase_and_avoids_shape_edit_fade() {
        let mut c=Config {enabled:true,mode:1,duration:1.0,..Config::default()};
        let mut e=Envelope::default();e.configure(c,1000.0);
        for _ in 0..180 {e.tick(true);}
        let phase=e.phase;assert!((phase-0.18).abs()<1e-9);
        c.duration=0.5;e.configure(c,1000.0);
        assert_eq!(e.phase,phase);assert_eq!(e.edit,1.0);
        e.tick(true);assert!((e.phase-phase-0.002).abs()<1e-9);
    }
    #[test]
    fn old_envelope_and_full_presets_validate_without_new_clock_fields() {
        let p=ModelFParams::default();let mut shape=EnvelopePreset::capture(&p);
        shape.values.remove("mseg_speed");shape.values.remove("mseg_fix_tempo");shape.validate().unwrap();
        let mut incomplete=EnvelopePreset {format:shape.format,values:shape.values.clone()};incomplete.values.remove("mseg_start");incomplete.values.insert("mseg_speed".into(),1.0);assert!(incomplete.validate().is_err());
        let mut full=crate::presets::Preset::capture(&p,"Legacy 0.2.1".into(),"Bass".into());
        full.values.remove("mseg_speed");full.values.remove("mseg_fix_tempo");assert_eq!(full.values.len(),172);full.validate().unwrap();
    }
}
