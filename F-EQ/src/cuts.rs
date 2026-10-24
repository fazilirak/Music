// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular.
use crate::dsp::{Coeff, Memory};
pub const SECTIONS: [usize; 5] = [1, 2, 3, 4, 8];
pub const SLOPES: [u32; 5] = [12, 24, 36, 48, 96];
#[derive(Debug)]
pub struct Cut {
    coeff: [[Coeff; 8]; 5],
    memory: [[[Memory; 8]; 5]; 2],
    pub weights: [f64; 5],
}
impl Default for Cut {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            coeff: [[Coeff::default(); 8]; 5],
            memory: [[[Memory::default(); 8]; 5]; 2],
            weights: [1., 0., 0., 0., 0.],
        }
    }
}
impl Cut {
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    pub fn reset(&mut self, index: usize) {
        self.memory = [[[Memory::default(); 8]; 5]; 2];
        self.weights = [0.; 5];
        self.weights[index] = 1.;
    }
    // configure: Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    pub fn configure(&mut self, rate: f64, hz: f64, highpass: bool) {
        for (bank, &n) in SECTIONS.iter().enumerate() {
            for i in 0..n {
                self.coeff[bank][i] = section(rate, hz, highpass, n, i);
            }
        }
    }
    // smooth alt adımı: Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular.
    pub fn smooth(&mut self, index: usize, t: f64) {
        for i in 0..5 {
            let target = if i == index { 1. } else { 0. };
            self.weights[i] += (target - self.weights[i]) * t;
        }
    }
    // Bir ses örneğini filtre/model hafızasından geçirir; bir sonraki örneğe taşınan durum bu adımda güncellenir.
    pub fn sample(&mut self, ch: usize, x: f64, mix: f64) -> f64 {
        let mut out = 0.;
        for (bank, &n) in SECTIONS.iter().enumerate() {
            let mut y = x;
            for i in 0..n {
                y = self.memory[ch][bank][i].tick(self.coeff[bank][i], y);
            }
            out += self.weights[bank] * y;
        }
        if mix == 0. { x } else { x + mix * (out - x) }
    }
}
// section alt adımı: Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular.
fn section(rate: f64, hz: f64, highpass: bool, n: usize, i: usize) -> Coeff {
    let q = 1. / (2. * (((2 * i + 1) as f64 * std::f64::consts::PI) / (4 * n) as f64).cos());
    Coeff::cut(rate, hz, highpass, q)
}
// magnitude alt adımı: Low/High Cut kademelerinin 12/24/36/48/96 dB eğim ve yumuşak geçiş durumunu uygular.
pub fn magnitude(rate: f64, hz: f64, f: f64, highpass: bool, index: usize, mix: f64) -> f64 {
    if mix == 0. {
        return 1.;
    }
    let n = SECTIONS[index];
    let mut re = 1.;
    let mut im = 0.;
    for i in 0..n {
        let (r, j) = section(rate, hz, highpass, n, i).complex(f, rate);
        let next = re * r - im * j;
        im = re * j + im * r;
        re = next;
    }
    ((1. + mix * (re - 1.)).powi(2) + (mix * im).powi(2)).sqrt()
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu all_orders_have_butterworth_corner_and_expected_rejection: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: magnitude → corner.log10 → abs → log10. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn all_orders_have_butterworth_corner_and_expected_rejection() {
        for index in 0..5 {
            for hp in [false, true] {
                let corner = magnitude(192000., 1000., 1000., hp, index, 1.);
                assert!((20. * corner.log10() + 3.0102999566).abs() < 1e-6);
                let rejection = 20.
                    * magnitude(192000., 1000., if hp { 250. } else { 4000. }, hp, index, 1.)
                        .log10();
                assert!(rejection < -(SLOPES[index] as f64 * 1.99));
            }
        }
    }
}
