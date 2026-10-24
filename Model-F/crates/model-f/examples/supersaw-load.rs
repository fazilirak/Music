// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: supersaw-load geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
//! Sustained seven-oscillator notes through the real engine, no deadline or dropped work.
use model_f::{
    engine::{Engine, Event, default_workers},
    params::ModelFParams,
};
use std::{fs, time::Instant};

// main alt adımı: supersaw-load geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → nth → and_then → ok. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main() {
    let target = std::env::args()
        .nth(2)
        .and_then(|s| s.parse::<usize>().ok())
        .unwrap_or(0)
        .min(3);
    let overload: f64 = std::env::args()
        .nth(3)
        .and_then(|s| s.parse().ok())
        .unwrap_or(0.0);
    let mut report = Vec::new();
    for block in [128usize, 256, 512] {
        for notes in [0usize, 1, 3, 4, 6, 8] {
            let mut c = ModelFParams::default().controls();
            c.feedback = model_f::params::overload_feedback(c.panel.emphasis, overload > 0.0);
            c.panel.ext_on = overload > 0.0;
            c.panel.ext_volume = overload / 10.0;
            c.poly = true;
            c.voices = 8;
            c.supersaw = Some(0.5);
            c.panel.osc[1].on = false;
            if target > 0 {
                c.mseg.enabled = true;
                c.mseg.mode = 1;
                c.mseg.target = target;
                c.mseg.amount = 0.3;
            }
            let mut engine = Engine::new();
            engine.set(&c);
            engine.prepare(48_000.0, 42);
            let workers = engine.start_workers(default_workers(), None);
            for key in 40..40 + notes as u8 {
                engine.event(Event::Note { key, on: true });
            }
            let (mut l, mut r) = (vec![0f32; block], vec![0f32; block]);
            for _ in 0..10 {
                engine.render(&[], &mut l, &mut r);
                engine.end_block(block);
            }
            let count = (48_000 / block).max(1);
            let mut times = Vec::with_capacity(count);
            let mut peak = 0f32;
            for _ in 0..count {
                let start = Instant::now();
                engine.render(&[], &mut l, &mut r);
                engine.end_block(block);
                times.push(start.elapsed().as_secs_f64() * 1000.0);
                for x in l.iter().chain(&r) {
                    assert!(x.is_finite());
                    peak = peak.max(x.abs());
                }
            }
            if notes > 0 {
                assert!(peak > 1e-5);
            }
            let period_ms = block as f64 / 48.0;
            let mean = times.iter().sum::<f64>() / count as f64;
            let over = times.iter().filter(|&&t| t > period_ms).count();
            times.sort_by(f64::total_cmp);
            let p95 = times[(count - 1) * 95 / 100];
            println!(
                "{block} samples, {notes:2} notes: mean {mean:.3}ms / {period_ms:.3}ms ({:.1}%), p95 {p95:.3}ms, over {over}/{count}, {workers} workers",
                mean / period_ms * 100.0
            );
            report.push(serde_json::json!({"buffer":block,"notes":notes,"oscillators":notes*7,"overload":overload,"mseg_target":target,"workers":workers,"sample_rate":48000,"mean_ms":mean,"period_ms":period_ms,"mean_budget_percent":mean/period_ms*100.0,"p95_ms":p95,"over_period":over,"blocks":count,"peak":peak}));
        }
    }
    let path = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "docs/supersaw-load.json".into());
    fs::write(path, serde_json::to_vec_pretty(&report).unwrap()).unwrap();
}
