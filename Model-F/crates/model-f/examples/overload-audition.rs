// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: overload-audition geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
//! Equal-RMS comparisons rendered through the actual voice engine.
use model_f::{
    engine::{Engine, Event},
    params::ModelFParams,
};
use model_f_core::preamp::InLoop;
use nih_plug::prelude::*;
use std::{fs, io::Write};
// wav alt adımı: overload-audition geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: fs::File::create → unwrap → x.len → f.write_all. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn wav(path: &str, x: &[f32]) {
    let mut f = fs::File::create(path).unwrap();
    let n = (x.len() * 2) as u32;
    f.write_all(b"RIFF").unwrap();
    f.write_all(&(n + 36).to_le_bytes()).unwrap();
    f.write_all(b"WAVEfmt ").unwrap();
    f.write_all(&16u32.to_le_bytes()).unwrap();
    f.write_all(&1u16.to_le_bytes()).unwrap();
    f.write_all(&1u16.to_le_bytes()).unwrap();
    f.write_all(&48000u32.to_le_bytes()).unwrap();
    f.write_all(&96000u32.to_le_bytes()).unwrap();
    f.write_all(&2u16.to_le_bytes()).unwrap();
    f.write_all(&16u16.to_le_bytes()).unwrap();
    f.write_all(b"data").unwrap();
    f.write_all(&n.to_le_bytes()).unwrap();
    for &v in x {
        f.write_all(&((v.clamp(-1., 1.) * 32767.) as i16).to_le_bytes())
            .unwrap();
    }
}
// Hazırlanmış durumdan ses veya panel çıktısını üretir; bu dosyanın DSP/GUI bağlamını aşağıdaki gövdeden ayırın.
// Gövdedeki başlıca çağrılar: FloatParam::new → Default::default → p.controls → Engine::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn render(old: bool, drive: f32, key: u8, resonance: f32, contour: f32) -> (Vec<f32>, f32) {
    let p = ModelFParams {
        emphasis: FloatParam::new(
            "Emphasis",
            resonance,
            FloatRange::Linear { min: 0., max: 10. },
        ),
        overload: FloatParam::new("Overload", drive, FloatRange::Linear { min: 0., max: 10. }),
        sustain: FloatParam::new("Sustain", 10., FloatRange::Linear { min: 0., max: 10. }),
        contour: FloatParam::new("Contour", contour, FloatRange::Linear { min: 0., max: 10. }),
        ..Default::default()
    };
    let mut c = p.controls();
    if old && drive > 0. {
        c.feedback = 1.;
    }
    let mut e = Engine::new();
    if old {
        e.set_in_loop(InLoop::Delayed);
    }
    e.set(&c);
    e.prepare(48000., 123);
    let (mut l, mut r) = ([0f32; 128], [0f32; 128]);
    let mut out = Vec::new();
    let mut lamp = 0f32;
    e.event(Event::Note { key, on: true });
    for b in 0..900 {
        if b == 700 {
            e.event(Event::Note { key, on: false });
        }
        e.render(&[], &mut l, &mut r);
        lamp = lamp.max(e.end_block(128));
        out.extend(l);
    }
    (out, lamp)
}
// bands alt adımı: overload-audition geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: let → exp → f64::from → powi. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn bands(x: &[f32]) -> (f64, f64, f64) {
    let (mut lo, mut hi, mut energy, mut bass, mut treble) = (0., 0., 0., 0., 0.);
    let a = 1. - (-std::f64::consts::TAU * 250. / 48000.).exp();
    let b = 1. - (-std::f64::consts::TAU * 2500. / 48000.).exp();
    for &x in x {
        let x = f64::from(x);
        lo += a * (x - lo);
        hi += b * (x - hi);
        energy += x * x;
        bass += lo * lo;
        treble += (x - hi).powi(2);
    }
    (
        (energy / x.len() as f64).sqrt(),
        bass / energy,
        treble / energy,
    )
}
// main alt adımı: overload-audition geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → nth → unwrap_or_else → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main() {
    let dir = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "docs/overload-audio".into());
    fs::create_dir_all(&dir).unwrap();
    let mut report = Vec::new();
    for key in [45, 57] {
        for drive in [5., 10.] {
            for (name, old, d) in [
                ("dry", false, 0.),
                ("previous", true, drive),
                ("warm", false, drive),
            ] {
                let (mut x, lamp) = render(old, d, key, 2., 0.);
                let (rms, bass, treble) = bands(&x[24000..84000]);
                let peak = x.iter().fold(0f32, |a, b| a.max(b.abs()));
                let gain = (0.16 / rms).min(0.89 / f64::from(peak));
                for x in &mut x {
                    *x *= gain as f32;
                }
                wav(&format!("{dir}/{key}-{drive:.0}-{name}.wav"), &x);
                report.push(serde_json::json!({"key":key,"drive":drive,"version":name,"raw_rms":rms,"bass_fraction":bass,"treble_fraction":treble,"lamp":lamp,"audition_gain":gain}));
            }
        }
    }
    fs::write(
        format!("{dir}/comparison.json"),
        serde_json::to_vec_pretty(&report).unwrap(),
    )
    .unwrap();
    println!("{}", serde_json::to_string_pretty(&report).unwrap());
}
