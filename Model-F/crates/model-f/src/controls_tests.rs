// Geliştirici: Fazıl Irak | Son düzenleme: 24.10.2026
// İki osilatör, bağımsız contour, Sync/FM ve envelope mod kilitlerini gerçek DSP ile sınar.
use crate::{engine::{Engine,Event},params::ModelFParams,mseg_ui,panel,presets,ModelF};
use nih_plug::{prelude::*,context::process::TestProcessContext};
use std::sync::Arc;
fn render(sync:bool,fm:f64,rate:f64,slave:bool)->Vec<f32> {
    let mut c=ModelFParams::default().controls(); c.sync=sync;c.fm=fm;
    c.interval=7.0;c.panel.osc[1].volume=0.0;c.panel.osc[1].on=false;
    c.panel.cutoff=1.5;c.panel.filter_contour.sustain=0.9;c.panel.loudness_contour.sustain=0.9;
    // Sync needs audible slave; FM must work with the second oscillator muted.
    if slave {c.panel.osc[1].on=true;c.panel.osc[1].volume=0.76;}
    let mut e=Engine::new();e.set(&c);e.prepare(rate,123);e.event(Event::Note{key:57,on:true});
    let (mut l,mut r)=([0f32;128],[0f32;128]);let mut out=Vec::new();
    for _ in 0..60 {e.render(&[],&mut l,&mut r);e.end_block(128);out.extend(l);}
    assert!(out.iter().all(|x|x.is_finite() && x.abs()<4.0));out
}
#[test]
fn fm_is_audible_with_muted_modulator_at_supported_rates() {
    for rate in [44100.0,48000.0,96000.0] {
        let dry=render(false,0.0,rate,false);let wet=render(false,0.45,rate,false);
        assert!(dry.iter().zip(wet).map(|(a,b)|f64::from(a-b).powi(2)).sum::<f64>()>0.01);
    }
}
#[test]
fn sync_changes_slave_waveform_without_instability() {
    let a=render(false,0.0,48000.0,true);let b=render(true,0.0,48000.0,true);
    assert!(a.iter().zip(b).map(|(a,b)|f64::from(a-b).powi(2)).sum::<f64>()>0.01);
}
#[test]
fn independent_controls_legacy_resolution_and_envelope_modes() {
    let p=Arc::new(ModelFParams::default());
    let ctx=TestProcessContext::<ModelF>::new(p.clone() as Arc<dyn Params>,48000.0,ProcessMode::Offline);
    let mut set=|id:&str,v:f32| {let q=p.get(id).unwrap();assert!(ctx.automate(q.as_ptr(),q.preview_normalized(v)));};
    set("attack",4.0);assert_eq!(p.effective("audio_attack"),4.0);
    set("audio_attack",0.0);set("osc2_waveform",0.0);set("waveform",2.0);
    set("audio_sustain",8.0);set("sustain",1.0);set("osc2_octave",4.0);
    let c=p.controls();assert_ne!(c.panel.osc[0].waveform,c.panel.osc[1].waveform);
    assert_ne!(c.panel.osc[0].range,c.panel.osc[1].range);
    assert_ne!(c.panel.filter_contour.attack,c.panel.loudness_contour.attack);
    assert_ne!(c.panel.filter_contour.sustain,c.panel.loudness_contour.sustain);
    set("mseg_amount",0.5);
    assert!(mseg_ui::enabled(&p,"mseg_attack"));assert!(!mseg_ui::enabled(&p,"mseg_duration"));
    assert!(panel::svg(&p,&Default::default()).contains(r##"fill="#e5e7df" stroke="#e5e7df""##));
    let mut state=mseg_ui::State::default();let (edits,_)=state.down(&p,400.0,300.0,false,keyboard_types::Modifiers::empty());assert!(edits.is_empty());
    set("mseg_enable",1.0);assert!(!mseg_ui::enabled(&p,"mseg_attack"));assert!(mseg_ui::enabled(&p,"mseg_duration"));
    let saved=presets::Preset::capture(&p,"Check".into(),"Bass".into());saved.validate().unwrap();
    assert!(saved.values["audio_decay"]>=0.0 && saved.values["osc2_waveform"]>=0.0);
}
#[test]
fn bank_has_active_ad_and_multi_sequences_across_every_category() {
    let bank=presets::factory();
    for category in presets::CATEGORIES {
        let sounds:Vec<_>=bank.iter().filter(|p|p.category==category).collect();
        for mode in [0.0,1.0] {assert!(sounds.iter().filter(|p|p.values["mseg_enable"]==mode && p.values["mseg_amount"].abs()>0.0).count()>=13);}
        assert!(sounds.iter().any(|p|p.values["osc_sync"]>0.5));assert!(sounds.iter().any(|p|p.values["fm_on"]>0.5));
        for p in sounds.iter().filter(|p|p.values["mseg_enable"]>0.5) {assert_eq!(p.values["mseg_mode"],1.0);for i in 0..16 {assert_eq!(p.values[&format!("mseg_p{i}_on")],1.0);}}
    }
    for t in 1..=3 {assert!(bank.iter().any(|p|p.values["mseg_enable"]>0.5 && p.values["mseg_target"]==t as f32));}
}
