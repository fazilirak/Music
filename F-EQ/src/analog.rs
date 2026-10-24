// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: F EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır.
//! Luftikus-inspired analogue behaviour. Gain error threshold, error range and
//! noise amplitude are ported from lkjb's MIT-licensed Luftikus (2019).
//! Our three serial shelves replace Luftikus's six parallel bands.
use std::sync::atomic::{AtomicU64, Ordering};
static SEED: AtomicU64 = AtomicU64::new(0x9134_125a_38e1);
#[derive(Debug)]
pub struct Color {
    rng: [u64; 2],
    pub offsets: [[f64; 3]; 2],
    target: [[f64; 3]; 2],
}
impl Default for Color {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        let seed = SEED.fetch_add(0x7919_34af, Ordering::Relaxed);
        Self {
            rng: [seed, seed ^ 0x5389_ad27],
            offsets: [[0.; 3]; 2],
            target: [[0.; 3]; 2],
        }
    }
}
impl Color {
    // JUCE Random's 48-bit LCG. Separate streams for the two audio channels.
    // unit alt adımı: F EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır.
    fn unit(&mut self, ch: usize) -> f64 {
        self.rng[ch] = self.rng[ch].wrapping_mul(0x5deece66d).wrapping_add(11) & 0xffff_ffff_ffff;
        (self.rng[ch] >> 16) as f64 / 4294967296.
    }
    // noise alt adımı: F EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır.
    pub fn noise(&mut self, ch: usize) -> f64 {
        1e-5 * (self.unit(ch) - 0.5)
    }
    // change: F EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    // Gövdedeki başlıca çağrılar: abs → self.unit. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn change(&mut self, old: [f64; 3], new: [f64; 3], enabled: bool) {
        for ch in 0..2 {
            for band in 0..3 {
                if old[band] != new[band] {
                    // Match setGain's 0.08 dB change threshold and ±0.04 dB error.
                    self.target[ch][band] = if enabled
                        && (new[band] - old[band] - self.target[ch][band]).abs() > 0.08
                    {
                        (self.unit(ch) - 0.5) * 0.08
                    } else {
                        0.
                    };
                }
            }
        }
    }
    // smooth alt adımı: F EQ’nun düşük seviyeli analog tolerans ve gürültü durumunu üretir/yumuşatır.
    pub fn smooth(&mut self, t: f64) {
        for ch in 0..2 {
            for b in 0..3 {
                self.offsets[ch][b] += (self.target[ch][b] - self.offsets[ch][b]) * t;
            }
        }
    }
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    pub fn reset(&mut self) {
        self.offsets = [[0.; 3]; 2];
        self.target = [[0.; 3]; 2];
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu noise_and_variation_match_original_ranges: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Color::default → c.change → c.smooth → c.offsets.iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn noise_and_variation_match_original_ranges() {
        let mut c = Color::default();
        c.change([0.; 3], [2.; 3], true);
        c.smooth(1.);
        assert!(c.offsets.iter().flatten().all(|x| x.abs() <= 0.04));
        assert_ne!(c.offsets[0], c.offsets[1]);
        let mut sum = 0.;
        let mut square = 0.;
        for _ in 0..100000 {
            let n = c.noise(0);
            assert!(n.abs() <= 5e-6);
            sum += n;
            square += n * n;
        }
        assert!((sum / 100000.).abs() < 5e-8);
        assert!((square / 100000f64).sqrt() > 2.8e-6);
    }
}
