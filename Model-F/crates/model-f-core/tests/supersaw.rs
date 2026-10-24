// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: supersaw regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
use model_f_core::{
    supersaw::{OFFSETS, detune_curve},
    voice::{Quality, Voice},
};

#[test]
// Regresyon senaryosu seven_circuit_oscillators_follow_measured_frequency_distribution: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Voice::prototype → voice.note → voice.tick → y.is_finite. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn seven_circuit_oscillators_follow_measured_frequency_distribution() {
    let rate = 48000.0;
    let mut voice = Voice::prototype(rate);
    voice.panel.quality = Quality::Potato;
    voice.supersaw = Some(1.0);
    voice.panel.cutoff = 0.8;
    voice.panel.contour_amount = 0.0;
    voice.note(69, true);
    let mut peak = 0f64;
    for _ in 0..48000 {
        let y = voice.tick();
        assert!(y.is_finite());
        peak = peak.max(y.abs());
    }
    let a = voice.supersaw_resets();
    for _ in 0..48000 {
        assert!(voice.tick().is_finite());
    }
    let b = voice.supersaw_resets();
    let hz: [f64; 7] = std::array::from_fn(|n| (b[n].1 - a[n].1) as f64 * rate / (b[n].0 - a[n].0));
    assert!(peak > 0.01);
    for n in 0..7 {
        let expected = if n == 3 {
            1.0
        } else {
            1.0 + OFFSETS[if n < 3 { n } else { n - 1 }] * detune_curve(1.0)
        };
        let cents = 1200.0 * (hz[n] / hz[3] / expected).log2();
        assert!(
            cents.abs() < 5.0,
            "oscillator {n}: {} Hz, expected ratio {expected}, error {cents} cents",
            hz[n]
        );
    }
}

#[test]
// Regresyon senaryosu detune_zero_and_mode_changes_remain_finite: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
fn detune_zero_and_mode_changes_remain_finite() {
    let mut v = Voice::prototype(44100.0);
    v.panel.quality = Quality::Potato;
    v.note(60, true);
    for detune in [Some(0.0), Some(0.5), Some(1.0), None, Some(0.75)] {
        v.supersaw = detune;
        for _ in 0..4096 {
            assert!(v.tick().is_finite());
        }
    }
}
