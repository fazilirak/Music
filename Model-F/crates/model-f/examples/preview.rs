// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
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
        "panel", "supersaw", "browser", "save", "about", "envelope", "overload", "ad-active", "multi-active", "envelope-multi", "envelope-tempo", "acid-resonance", "tuning-432", "tuning-440", "tuning-442", "tuning-444", "tuning-528", "tuning-custom",
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
        let params=if name=="ad-active" {ModelFParams {mseg_amount: FloatParam::new("Amount",0.5,FloatRange::Linear{min:-1.0,max:1.0}),..params}} else {params};
        let params=if name=="multi-active" || name=="envelope-multi" {ModelFParams {mseg_enable: FloatParam::new("Multi",1.0,FloatRange::Linear{min:0.0,max:1.0}),..params}} else {params};
        let params=if name=="envelope-tempo" {ModelFParams {
            mseg_enable:FloatParam::new("Multi",1.0,FloatRange::Linear{min:0.0,max:1.0}),
            mseg_speed:FloatParam::new("Speed",5.0,FloatRange::Linear{min:0.0,max:6.0}),
            mseg_fix_tempo:FloatParam::new("Fix Tempo",1.0,FloatRange::Linear{min:0.0,max:1.0}),..params}} else {params};
        let params=if name=="acid-resonance" {ModelFParams {
            character:FloatParam::new("Character",4.0,FloatRange::Linear{min:0.0,max:5.0}),
            emphasis:FloatParam::new("Emphasis",8.6,FloatRange::Linear{min:0.0,max:10.0}),..params}} else {params};
        let tuning=match name {"tuning-432"=>Some(432.0),"tuning-440"=>Some(440.0),"tuning-442"=>Some(442.0),"tuning-444"=>Some(444.0),"tuning-528"=>Some(528.0),"tuning-custom"=>Some(439.1),_=>None};
        let params=if let Some(hz)=tuning {ModelFParams {global_tuning:FloatParam::new("Global Tuning",hz,FloatRange::Linear{min:model_f::params::GLOBAL_TUNING_MIN,max:model_f::params::GLOBAL_TUNING_MAX}).with_step_size(0.1),..params}} else {params};
        view.overload = if name == "overload" { 1.0 } else { 0.0 };
        view.envelope = name == "envelope" || name == "envelope-multi" || name=="envelope-tempo";
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
