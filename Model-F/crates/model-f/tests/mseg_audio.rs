// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: mseg_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
//! Verify the three modulation destinations in the actual circuit engine.
use model_f::{
    engine::{Engine, Event},
    params::ModelFParams,
};
use model_f_core::voice::Waveform;

// sound alt adımı: mseg_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Gövdedeki başlıca çağrılar: ModelFParams::default → controls → Engine::new → e.set. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn sound(target: usize, wet: bool) -> Vec<f32> {
    let mut c = ModelFParams::default().controls();
    c.panel.osc[0].waveform = Waveform::Square;
    c.panel.osc[1].waveform = Waveform::Square;
    c.panel.cutoff = 0.35;
    c.mseg.enabled = true;
    c.mseg.mode = 1;
    c.mseg.duration = 0.1;
    c.mseg.target = if wet { target } else { 1 };
    c.mseg.amount = if wet { 0.2 } else { 0.0 };
    let mut e = Engine::new();
    e.set(&c);
    e.prepare(48000., 123);
    e.event(Event::Note { key: 57, on: true });
    let (mut l, mut r) = ([0f32; 128], [0f32; 128]);
    let mut out = Vec::new();
    for _ in 0..60 {
        e.render(&[], &mut l, &mut r);
        e.end_block(128);
        out.extend(l);
    }
    out
}
#[test]
// Regresyon senaryosu every_destination_changes_audible_circuit_output: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: sound → wet.iter → all → x.is_finite. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn every_destination_changes_audible_circuit_output() {
    let dry = sound(0, false);
    for target in 1..=3 {
        let wet = sound(target, true);
        assert!(wet.iter().all(|x| x.is_finite()), "target {target}");
        let energy: f64 = wet.iter().map(|x| f64::from(*x).powi(2)).sum();
        let difference: f64 = wet
            .iter()
            .zip(&dry)
            .map(|(a, b)| f64::from(a - b).powi(2))
            .sum();
        assert!(
            energy > 0.001 && difference > 0.001,
            "target {target}: energy {energy}, difference {difference}"
        );
    }
}
