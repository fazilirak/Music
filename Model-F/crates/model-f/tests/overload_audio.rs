// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: overload_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
//! Audible feedback, real overload metering and stability in each voice mode.
use model_f::{
    engine::{Engine, Event},
    params::ModelFParams,
};
use nih_plug::prelude::*;
// Host’un parametre nesnesine/sözleşmesine erişimini sağlar; arayüz metniyle kalıcı kimliği ayırır.
fn params(mode: f32, drive: f32) -> ModelFParams {
    ModelFParams {
        mode: FloatParam::new("Mode", mode, FloatRange::Linear { min: 0., max: 6. }),
        overload: FloatParam::new("Overload", drive, FloatRange::Linear { min: 0., max: 10. }),
        sustain: FloatParam::new("Sustain", 10., FloatRange::Linear { min: 0., max: 10. }),
        ..ModelFParams::default()
    }
}
// Regresyon senaryosu sound: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: params → controls → Engine::new → e.set. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn sound(mode: f32, drive: f32) -> (Vec<f32>, f32) {
    let c = params(mode, drive).controls();
    let mut e = Engine::new();
    e.set(&c);
    e.prepare(48000., 123);
    let (mut l, mut r) = ([0f32; 128], [0f32; 128]);
    for _ in 0..20 {
        e.render(&[], &mut l, &mut r);
        assert_eq!(e.end_block(128), 0.);
    }
    for key in 48..48
        + if mode == 6. {
            1
        } else if mode == 1. {
            8
        } else {
            4
        }
    {
        e.event(Event::Note { key, on: true });
    }
    let mut out = Vec::new();
    let mut lamp = 0f32;
    for _ in 0..200 {
        e.render(&[], &mut l, &mut r);
        lamp = lamp.max(e.end_block(128));
        assert!(l.iter().chain(&r).all(|v| v.is_finite() && v.abs() < 4.));
        out.extend(l);
    }
    e.event(Event::AllSoundOff);
    for _ in 0..250 {
        e.render(&[], &mut l, &mut r);
        e.end_block(128);
    }
    assert!(l.iter().chain(&r).all(|v| v.abs() < 1e-5));
    assert!(e.lamp() < 0.001);
    (out, lamp)
}
#[test]
// Regresyon senaryosu feedback_changes_sound_and_lights_the_real_detector: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: let → sound → iter → zip. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn feedback_changes_sound_and_lights_the_real_detector() {
    for mode in [6., 0., 1.] {
        let (dry, dark) = sound(mode, 0.);
        assert_eq!(dark, 0.);
        for drive in [5., 10.] {
            let (wet, lamp) = sound(mode, drive);
            let diff: f64 = wet
                .iter()
                .zip(&dry)
                .map(|(a, b)| f64::from(a - b).powi(2))
                .sum();
            println!("mode={mode}, drive={drive}, lamp={lamp}, difference={diff}");
            assert!(diff > 0.01);
            let mut a = 0f64;
            let mut b = 0f64;
            let mut cross = 0f64;
            for (&x, &y) in dry[8192..].iter().zip(&wet[8192..]) {
                a += f64::from(x).powi(2);
                b += f64::from(y).powi(2);
                cross += f64::from(x) * f64::from(y);
            }
            let matched_difference = (2. - 2. * cross / (a * b).sqrt()).max(0.).sqrt();
            assert!(
                matched_difference > if drive == 5. { 0.12 } else { 0.30 },
                "mode {mode}, drive {drive}: level-matched difference {matched_difference}"
            );
            if drive == 10. {
                assert!(lamp > 0.1, "mode {mode}: lamp {lamp}");
            }
        }
    }
}

// attack_sound alt adımı: overload_audio regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Gövdedeki başlıca çağrılar: params → FloatParam::new → p.controls → Engine::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn attack_sound(drive: f32, emphasis: f32) -> Vec<f32> {
    let mut p = params(6., drive);
    p.emphasis = FloatParam::new(
        "Emphasis",
        emphasis,
        FloatRange::Linear { min: 0., max: 10. },
    );
    let c = p.controls();
    let mut e = Engine::new();
    e.set(&c);
    e.prepare(48000., 321);
    let (mut l, mut r) = ([0f32; 128], [0f32; 128]);
    let mut out = Vec::new();
    // Include retriggers after short rests, not only a fresh voice.
    for block in 0..100 {
        if block % 32 == 0 {
            e.event(Event::Note { key: 45, on: true });
        }
        if block % 32 == 24 {
            e.event(Event::Note { key: 45, on: false });
        }
        e.render(&[], &mut l, &mut r);
        e.end_block(128);
        out.extend(l);
    }
    out
}
#[test]
// Regresyon senaryosu overload_does_not_blow_up_resonant_note_attacks: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: attack_sound → iter → fold → a.max. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn overload_does_not_blow_up_resonant_note_attacks() {
    for emphasis in [0., 5., 8., 10.] {
        let dry = attack_sound(0., emphasis);
        let wet = attack_sound(10., emphasis);
        for start in [0, 4096, 8192] {
            let peak = |s: &[f32]| {
                s[start..start + 2400]
                    .iter()
                    .fold(0f32, |a, b| a.max(b.abs()))
            };
            let d = peak(&dry);
            let w = peak(&wet);
            println!("resonance {emphasis}, retrigger {start}: dry {d}, overload {w}");
            assert!(
                w <= d * 1.6 + 0.001,
                "resonance {emphasis}: attack overshoot {w}/{d}"
            );
        }
    }
}
#[test]
// Regresyon senaryosu zero_overload_preserves_the_original_dry_path: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: params → controls → Engine::new → a.set. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn zero_overload_preserves_the_original_dry_path() {
    use model_f_core::preamp::InLoop;
    let c = params(6., 0.).controls();
    let mut a = Engine::new();
    let mut b = Engine::new();
    a.set(&c);
    b.set(&c);
    b.set_in_loop(InLoop::Delayed);
    a.prepare(48000., 123);
    b.prepare(48000., 123);
    a.event(Event::Note { key: 45, on: true });
    b.event(Event::Note { key: 45, on: true });
    let (mut al, mut ar, mut bl, mut br) = ([0f32; 128], [0f32; 128], [0f32; 128], [0f32; 128]);
    for _ in 0..80 {
        a.render(&[], &mut al, &mut ar);
        b.render(&[], &mut bl, &mut br);
        a.end_block(128);
        b.end_block(128);
        assert_eq!(al, bl);
        assert_eq!(ar, br);
    }
}
