// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: F-EQ doygunluk eğrisi ve antitürev tabanlı sayısal işleme adımıdır.
//! Independent saturation, not part of Luftikus's original Analog mode.
//! First-order antiderivative antialiasing of only the nonlinear residual:
//! the clean component stays undelayed and amount=0 is exactly transparent.
#[derive(Default, Debug, Clone, Copy)]
pub struct Saturator {
    previous: f64,
}
impl Saturator {
    // Bir ses örneğini filtre/model hafızasından geçirir; bir sonraki örneğe taşınan durum bu adımda güncellenir.
    // Gövdedeki başlıca çağrılar: integral → abs → log → exp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn sample(&mut self, x: f64, amount: f64) -> f64 {
        let old = self.previous;
        self.previous = x;
        if amount == 0. {
            return x;
        }
        let k = 1. + 5. * amount;
        // integral alt adımı: F-EQ doygunluk eğrisi ve antitürev tabanlı sayısal işleme adımıdır.
        fn integral(x: f64, k: f64) -> f64 {
            let v = (k * x).abs();
            // Stable log(cosh), including input peaks above full scale.
            (v + (-2. * v).exp().ln_1p() - std::f64::consts::LN_2) / (k * k) - 0.5 * x * x
        }
        let delta = x - old;
        let residual = if delta.abs() > 1e-5 * (1. + x.abs() + old.abs()) {
            (integral(x, k) - integral(old, k)) / delta
        } else {
            let mid = (x + old) * 0.5;
            (k * mid).tanh() / k - mid
        };
        x + amount * residual
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu zero_is_exact_and_drive_softens_peaks: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn zero_is_exact_and_drive_softens_peaks() {
        let mut s = Saturator::default();
        for x in [-2., -0.2, 0., 0.3, 2.] {
            assert_eq!(s.sample(x, 0.), x);
        }
        for _ in 0..20 {
            s.sample(1., 1.);
        }
        let y = s.sample(1., 1.);
        assert!(y > 0.16 && y < 0.17);
        for x in [-100., 100., 0., 1e-10] {
            assert!(s.sample(x, 1.).is_finite());
        }
    }
    #[test]
    // Regresyon senaryosu generates_third_harmonic: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn generates_third_harmonic() {
        let mut s = Saturator::default();
        let mut third = 0.;
        let n = 48000;
        for i in 0..n {
            let phase = 2. * std::f64::consts::PI * 1000. * i as f64 / 48000.;
            let y = s.sample(0.5 * phase.sin(), 0.8);
            third += y * (3. * phase).sin();
        }
        assert!((third * 2. / n as f64).abs() > 0.01);
    }
}
