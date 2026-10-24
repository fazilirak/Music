// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Yedi osilatörün frekans dağılımı ve detune eğrisini uygular; osilatör sayısı sabittir.
//! Model F: seven Model F VCOs, one ladder/VCA. Measured JP-8000 distribution
//! and detune fit: Adam Szabo, How to Emulate the Super Saw (2010), pp. 9–15.
//! https://www.adamszabo.com/internet/adam_szabo_how_to_emulate_the_super_saw.pdf
use crate::{tuning::Drive, vco::Vco, voice::Quality};
pub const OFFSETS: [f64; 6] = [
    -0.11002313,
    -0.06288439,
    -0.01952356,
    0.01991221,
    0.06216538,
    0.10745242,
];
// detune_curve alt adımı: Yedi osilatörün frekans dağılımı ve detune eğrisini uygular; osilatör sayısı sabittir.
// Gövdedeki başlıca çağrılar: x.clamp → into_iter → fold → clamp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn detune_curve(x: f64) -> f64 {
    let x = x.clamp(0.0, 1.0);
    let coefficients: [f64; 12] = [
        10028.7312891634,
        -50818.8652045924,
        111363.4808729368,
        -138150.6761080548,
        106649.6679158292,
        -53046.9642751875,
        17019.9518580080,
        -3425.0836591318,
        404.2703938388,
        -24.1878824391,
        0.6717417634,
        0.0030115596,
    ];
    coefficients
        .into_iter()
        .fold(0.0, |y, c| y * x + c)
        .clamp(0.0, 1.0)
}
#[derive(Debug, Clone)]
pub struct SuperSaw {
    sides: [Vco; 6],
    detune: f64,
    octaves: [f64; 6],
    last_reset: f64,
    last_count: u64,
    frequency: f64,
    low: f64,
    hp_alpha: f64,
}
impl SuperSaw {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: std::array::from_fn → prototype.clone → v.reset → v.tick_current. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn new(prototype: &Vco) -> Self {
        let sides = std::array::from_fn(|n| {
            let mut v = prototype.clone();
            v.reset();
            // Distinct free-running phases; no per-note reset or RT allocation.
            for _ in 0..[37, 211, 89, 307, 151, 433][n] {
                v.tick_current(1e-5, 1e-8, 0.0);
            }
            v
        });
        Self {
            sides,
            detune: -1.0,
            octaves: [0.0; 6],
            last_reset: 0.0,
            last_count: 0,
            frequency: 110.0,
            low: 0.0,
            hp_alpha: 0.01,
        }
    }
    // İşleme akışını bir örnek/adım ilerletir; hafıza güncelleme sırası duyulan çıktıyı etkiler.
    // Gövdedeki başlıca çağrılar: detune_curve → OFFSETS.map → libm::log2 → clamp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn tick(
        &mut self,
        center: f64,
        reset: (f64, u64),
        detune: f64,
        center_drive: Drive,
        octave_step: f64,
        rate: f64,
        quality: Quality,
    ) -> f64 {
        if detune != self.detune {
            self.detune = detune;
            let curve = detune_curve(detune);
            self.octaves = OFFSETS.map(|o| libm::log2(1.0 + o * curve));
        }
        if reset.1 != self.last_count {
            let period = reset.0 - self.last_reset;
            if period > 1.0 {
                self.frequency = (rate / period).clamp(10.0, rate * 0.3);
            }
            self.hp_alpha = 1.0 - libm::exp(-std::f64::consts::TAU * self.frequency / rate);
            self.last_reset = reset.0;
            self.last_count = reset.1;
        }
        // Measured mix law fixed at .75: one detune control, exactly seven saws.
        const MIX: f64 = 0.75;
        const CENTER: f64 = -0.55366 * MIX + 0.99785;
        const SIDE: f64 = -0.73764 * MIX * MIX + 1.2841 * MIX + 0.044372;
        let mut sum = center * CENTER;
        for (n, v) in self.sides.iter_mut().enumerate() {
            let mut drive = center_drive;
            drive.inputs[5].v += self.octaves[n] * octave_step;
            let i = drive.apply(&mut v.expo);
            v.set_oversample(if quality == Quality::Potato { 1 } else { 2 });
            v.table_on = quality == Quality::Potato;
            let o = v.tick(i, 0.0);
            let (saw, _) =
                crate::voice::waveform_source(crate::voice::Waveform::Sawtooth, &o, None);
            sum += saw * SIDE;
        }
        sum /= libm::sqrt(CENTER * CENTER + 6.0 * SIDE * SIDE);
        self.low += self.hp_alpha * (sum - self.low);
        (sum - self.low) * std::f64::consts::SQRT_2
    }
    // resets alt adımı: Yedi osilatörün frekans dağılımı ve detune eğrisini uygular; osilatör sayısı sabittir.
    pub fn resets(&self) -> [(f64, u64); 6] {
        std::array::from_fn(|n| self.sides[n].last_reset())
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu measured_nonlinear_detune_has_the_jp_knee: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn measured_nonlinear_detune_has_the_jp_knee() {
        assert!((detune_curve(0.0) - 0.0030115596).abs() < 1e-9);
        assert!(detune_curve(0.5) < 0.12);
        assert!(detune_curve(0.9) < 0.4);
        assert!(detune_curve(1.0) > 0.99);
        assert_eq!(OFFSETS.len() + 1, 7);
        assert!(OFFSETS[0].abs() > OFFSETS[5]);
    }
}
