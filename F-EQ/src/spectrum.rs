// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: F EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir.
//! Audio only publishes stereo samples to atomic storage. Windowing, FFT and
//! logarithmic binning run on the GUI thread; no locks or audio allocations.
use std::f64::consts::PI;
use std::sync::atomic::{AtomicU64, AtomicUsize, Ordering};
pub const BINS: usize = 96;
pub const FFT_SIZE: usize = 4096;
const RING: usize = 8192;
pub struct Analyzer {
    samples: [AtomicU64; RING],
    cursor: AtomicUsize,
}
impl Default for Analyzer {
    // Varsayılan değerleri kurar; parametre aralığı/preset varsayılanıyla birlikte değişmelidir.
    fn default() -> Self {
        Self {
            samples: std::array::from_fn(|_| AtomicU64::new(0)),
            cursor: AtomicUsize::new(0),
        }
    }
}
#[derive(Clone, Copy, Default)]
struct Complex {
    re: f64,
    im: f64,
}
// fft alt adımı: F EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir.
// Gövdedeki başlıca çağrılar: a.swap → let → angle.cos → angle.sin. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn fft(a: &mut [Complex; FFT_SIZE]) {
    let mut j = 0;
    for i in 1..FFT_SIZE {
        let mut bit = FFT_SIZE >> 1;
        while j & bit != 0 {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if i < j {
            a.swap(i, j);
        }
    }
    let mut len = 2;
    while len <= FFT_SIZE {
        let angle = -2. * PI / len as f64;
        let (wr, wi) = (angle.cos(), angle.sin());
        for start in (0..FFT_SIZE).step_by(len) {
            let (mut r, mut im) = (1., 0.);
            for k in 0..len / 2 {
                let u = a[start + k];
                let b = a[start + k + len / 2];
                let v = Complex {
                    re: b.re * r - b.im * im,
                    im: b.re * im + b.im * r,
                };
                a[start + k] = Complex {
                    re: u.re + v.re,
                    im: u.im + v.im,
                };
                a[start + k + len / 2] = Complex {
                    re: u.re - v.re,
                    im: u.im - v.im,
                };
                let next = r * wr - im * wi;
                im = r * wi + im * wr;
                r = next;
            }
        }
        len *= 2;
    }
}
impl Analyzer {
    // push alt adımı: F EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir.
    pub fn push(&self, left: f32, right: f32) {
        let n = self.cursor.load(Ordering::Relaxed);
        let pair = left.to_bits() as u64 | ((right.to_bits() as u64) << 32);
        self.samples[n % RING].store(pair, Ordering::Relaxed);
        self.cursor.store(n.wrapping_add(1), Ordering::Release);
    }
    // cursor alt adımı: F EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir.
    pub fn cursor(&self) -> usize {
        self.cursor.load(Ordering::Acquire)
    }
    // Örnek/filtre/faz gibi çalışma hafızasını yeniden başlangıca alır; parametre tanımıyla aynı işlem değildir.
    pub fn reset(&self) {
        self.cursor.store(0, Ordering::Release);
    }
    // read: F EQ çıkış spektrumunun tampon, FFT ve atomik okunabilir gösterimini üretir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    // Gövdedeki başlıca çağrılar: self.cursor → Complex::default → load → cos. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn read(&self, rate: f64) -> [f32; BINS] {
        let end = self.cursor();
        if end < FFT_SIZE {
            return [-100.; BINS];
        }
        let mut left = [Complex::default(); FFT_SIZE];
        let mut right = left;
        for i in 0..FFT_SIZE {
            let pair = self.samples[(end - FFT_SIZE + i) % RING].load(Ordering::Relaxed);
            let w = 0.5 - 0.5 * (2. * PI * i as f64 / FFT_SIZE as f64).cos();
            let l = f32::from_bits(pair as u32);
            let r = f32::from_bits((pair >> 32) as u32);
            left[i].re = if l.is_finite() { l as f64 * w } else { 0. };
            right[i].re = if r.is_finite() { r as f64 * w } else { 0. };
        }
        // If the writer lapped this snapshot, discard it instead of displaying
        // a mixture of different time windows. All individual accesses are atomic.
        if self.cursor().wrapping_sub(end) > RING - FFT_SIZE {
            return [-100.; BINS];
        }
        fft(&mut left);
        fft(&mut right);
        let mut power = [0.; FFT_SIZE / 2 + 1];
        let scale = (4. / FFT_SIZE as f64).powi(2);
        for k in 1..power.len() {
            power[k] = (left[k].re.powi(2)
                + left[k].im.powi(2)
                + right[k].re.powi(2)
                + right[k].im.powi(2))
                * 0.5
                * scale;
        }
        std::array::from_fn(|i| {
            let lo = 20. * 1000f64.powf(i as f64 / BINS as f64);
            let hi = 20. * 1000f64.powf((i + 1) as f64 / BINS as f64);
            if lo >= rate / 2. {
                return -100.;
            }
            let first = (lo * FFT_SIZE as f64 / rate).round().max(1.) as usize;
            let last = (hi * FFT_SIZE as f64 / rate)
                .round()
                .max(first as f64)
                .min((FFT_SIZE / 2) as f64) as usize;
            let peak = power[first.min(FFT_SIZE / 2)..=last]
                .iter()
                .copied()
                .fold(0f64, f64::max);
            (10. * peak.max(1e-12).log10()).clamp(-100., 0.) as f32
        })
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu sine_frequency_and_level_survive_opposite_stereo_polarity: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Analyzer::default → sin → a.push → a.read. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn sine_frequency_and_level_survive_opposite_stereo_polarity() {
        let a = Analyzer::default();
        let frequency = 48000. * 50. / FFT_SIZE as f64; // 585.9375 Hz, exactly bin 50 at 48k.
        for i in 0..FFT_SIZE * 2 {
            let x = (2. * PI * frequency * i as f64 / 48000.).sin() as f32 * 0.5;
            a.push(x, -x);
        }
        let bins = a.read(48000.);
        let peak = bins.iter().copied().fold(-100f32, f32::max);
        assert!((peak + 6.0206).abs() < 0.05);
        let index = bins.iter().position(|v| *v == peak).unwrap();
        let lo = 20. * 1000f64.powf(index as f64 / BINS as f64);
        let hi = 20. * 1000f64.powf((index + 1) as f64 / BINS as f64);
        assert!(frequency >= lo * 0.95 && frequency <= hi * 1.05);
    }
    #[test]
    // Regresyon senaryosu silence_and_reset_have_no_spurious_spectrum: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn silence_and_reset_have_no_spurious_spectrum() {
        let a = Analyzer::default();
        for _ in 0..FFT_SIZE {
            a.push(0., 0.);
        }
        assert_eq!(a.read(48000.), [-100.; BINS]);
        a.reset();
        assert_eq!(a.read(48000.), [-100.; BINS]);
    }
}
