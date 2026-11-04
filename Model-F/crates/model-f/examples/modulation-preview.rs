use model_f::{panel,params::ModelFParams,mod_view::View,ModelF};
use nih_plug::{prelude::*,context::process::TestProcessContext};
use std::{sync::Arc,path::PathBuf};
fn main() {
    let dir=PathBuf::from(std::env::args().nth(1).expect("output directory"));std::fs::create_dir_all(&dir).unwrap();
    let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
    let set=|id:&str,value:f32| {let par=p.get(id).unwrap();assert!(ctx.automate(par.as_ptr(),par.preview_normalized(value)));};
    set("mod_system",1.);
    for page in 0..6 {let v=View::bank(&p,page);let setv=|id:&str,value:f32|set(&v.real_id(id),value);
        setv("mseg_enable",1.);setv("mseg_amount",0.65);setv("mseg_mode",1.);
        setv("lfo_status",1.);setv("lfo_freq",0.75+page as f32*0.25);setv("lfo_depth",0.12);
        setv("step_status",1.);setv("step_depth",0.3);setv("step_retrigger",1.);
        for n in 0..16 {setv(&format!("step_{n}"),[0.9,0.25,0.6,0.45,0.75,0.2,0.4,1.][(n+page)%8]);}
        let mut ui=panel::View {envelope:true,..Default::default()};ui.env.page=page;ui.env.selected=2;
        let svg=panel::svg(&p,&ui);let name=model_f::modulators::LABELS[page].to_lowercase().replace(' ',"-");
        std::fs::write(dir.join(format!("modulators-{name}.svg")),&svg).unwrap();
        panel::render(&svg,panel::W,panel::H).unwrap().save_png(dir.join(format!("modulators-{name}.png"))).unwrap();
    }
    let svg=panel::svg(&p,&panel::View {name:"Model F 0.3.0 — Modulators".into(),..Default::default()});
    std::fs::write(dir.join("preamp-main.svg"),&svg).unwrap();
    panel::render(&svg,panel::W,panel::H).unwrap().save_png(dir.join("preamp-main.png")).unwrap();
    for page in 0..6 {let v=View::bank(&p,page);for id in ["mseg_enable","lfo_status","step_status"] {set(&v.real_id(id),0.);}}
    let svg=panel::svg(&p,&panel::View {name:"Model F 0.3.0 — Modulators".into(),..Default::default()});
    panel::render(&svg,panel::W,panel::H).unwrap().save_png(dir.join("modulators-off-main.png")).unwrap();
    set("mod_cutoff_lfo_status",1.);
    let mut ui=panel::View {envelope:true,..Default::default()};ui.env.page=1;
    let svg=panel::svg(&p,&ui);
    panel::render(&svg,panel::W,panel::H).unwrap().save_png(dir.join("modulators-activity-states.png")).unwrap();
    let preset=model_f::presets::factory().into_iter().find(|p|p.name=="Sub Foundation").unwrap();
    for (id,par) in p.list() {set(id,preset.values.get(id).copied().unwrap_or_else(||par.default_plain_value()));}
    for page in [0,2] {
        let mut ui=panel::View {name:preset.name.clone(),status:format!("Loaded: {} / {}",preset.name,preset.category),envelope:true,..Default::default()};ui.env.page=page;
        let svg=panel::svg(&p,&ui);
        panel::render(&svg,panel::W,panel::H).unwrap().save_png(dir.join(format!("sub-foundation-page-{page}.png"))).unwrap();
    }
}
