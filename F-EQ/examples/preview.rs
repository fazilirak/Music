// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
use simple_eq::{panel, params::EqParams};
// main alt adımı: preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → nth → unwrap_or → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main() {
    let dir = std::env::args().nth(1).unwrap_or("docs".into());
    std::fs::create_dir_all(&dir).unwrap();
    let p = EqParams::default();
    let svg = panel::svg(&p, 0., 48000.);
    panel::render(
        &panel::with_about(&svg),
        panel::W * 2,
        panel::H * 2,
        &panel::options(),
    )
    .unwrap()
    .save_png(format!("{dir}/about.png"))
    .unwrap();
    std::fs::write(format!("{dir}/panel.svg"), &svg).unwrap();
    panel::render(&svg, panel::W * 2, panel::H * 2, &panel::options())
        .unwrap()
        .save_png(format!("{dir}/panel.png"))
        .unwrap();
    // A second image uses actual FFT data from an explicit example signal.
    let analyzer = simple_eq::spectrum::Analyzer::default();
    for i in 0..8192 {
        let mut x = 0.;
        for (f, a) in [
            (60., 0.2),
            (140., 0.14),
            (320., 0.09),
            (850., 0.07),
            (1800., 0.04),
            (4300., 0.02),
            (11000., 0.012),
        ] {
            x += a * (2. * std::f64::consts::PI * f * i as f64 / 48000.).sin();
        }
        analyzer.push(x as f32, x as f32);
    }
    let bins = analyzer.read(48000.);
    let svg = panel::svg_with_spectrum(&p, 0.3, 48000., &bins);
    panel::render(&svg, panel::W * 2, panel::H * 2, &panel::options())
        .unwrap()
        .save_png(format!("{dir}/analyzer-preview.png"))
        .unwrap();
}
