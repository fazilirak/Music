// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
use model_f::{
    panel::{self, View},
    params::ModelFParams,
    presets,
};
use nih_plug::prelude::*;
// main alt adımı: preview geliştirme örneğinin çıktı/ölçüm girişidir; ürün audio callback’i değildir.
// Gövdedeki başlıca çağrılar: std::env::args → nth → unwrap_or_else → into. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main() {
    let dir = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "docs/model-f".into());
    std::fs::create_dir_all(&dir).expect("preview directory");
    let mut view = View {
        name: "Init — Model F".into(),
        category: "Init".into(),
        status: "Ready. Mono is the default voice mode.".into(),
        filter: "Bass".into(),
        ..View::default()
    };
    for name in [
        "panel", "supersaw", "browser", "save", "about", "envelope", "overload",
    ] {
        let params = if name == "supersaw" {
            ModelFParams {
                mode: FloatParam::new("Voice Mode", 1.0, FloatRange::Linear { min: 0.0, max: 6.0 })
                    .with_step_size(1.0),
                ..ModelFParams::default()
            }
        } else {
            ModelFParams::default()
        };
        let params = if name == "overload" {
            ModelFParams {
                overload: FloatParam::new(
                    "Overload",
                    10.0,
                    FloatRange::Linear {
                        min: 0.0,
                        max: 10.0,
                    },
                ),
                ..params
            }
        } else {
            params
        };
        view.overload = if name == "overload" { 1.0 } else { 0.0 };
        view.envelope = name == "envelope";
        view.env.selected = 1;
        view.about = name == "about";
        view.browser = name == "browser";
        view.save = name == "save";
        view.save_name = "My first sound".into();
        view.rows = presets::factory()
            .into_iter()
            .filter(|p| p.category == "Bass")
            .collect();
        let svg = panel::svg(&params, &view);
        std::fs::write(format!("{dir}/{name}.svg"), &svg).expect("SVG");
        panel::render(&svg, panel::W, panel::H)
            .expect("render")
            .save_png(format!("{dir}/{name}.png"))
            .expect("PNG");
    }
}
