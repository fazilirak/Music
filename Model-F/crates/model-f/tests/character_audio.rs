// SPDX-License-Identifier: GPL-3.0-or-later
// Geliştirici: Fazıl Irak | Düzenleme: 26.10.2026
//! Yeni karakterlerin gerçek Engine yolu, kayıt/preset ve nota akorduyla regresyonları.
#![allow(unsafe_code)]
use model_f::{engine::{Controls,Engine,Event},params::ModelFParams,presets,panel,ModelF};
use nih_plug::{prelude::*,context::process::TestProcessContext};
use std::sync::Arc;
struct TestGui(Arc<ModelFParams>);
impl GuiContext for TestGui {
    fn plugin_api(&self)->nih_plug::context::PluginApi {nih_plug::context::PluginApi::Standalone}
    fn request_resize(&self)->bool {true}
    // SAFETY: yalnız bu testin sahibi olduğu parametre işaretçileri kabul edilir.
    unsafe fn raw_begin_set_parameter(&self,_:ParamPtr) {}
    unsafe fn raw_end_set_parameter(&self,_:ParamPtr) {}
    unsafe fn raw_set_parameter_normalized(&self,p:ParamPtr,v:f32) {
        let ctx=TestProcessContext::<ModelF>::new(self.0.clone() as Arc<dyn Params>,48000.0,ProcessMode::Offline);
        assert!(ctx.automate(p,v));
    }
    fn get_state(&self)->PluginState {panic!("not used by preset apply")}
    fn set_state(&self,_:PluginState) {panic!("not used by preset apply")}
}
fn render(c:&Controls,rate:f64,block:usize)->Vec<f32> {
    let mut engine=Engine::new();engine.set(c);engine.prepare(rate,42);
    engine.event(Event::Note{key:45,on:true});
    let mut l=vec![0.0;block];let mut r=l.clone();let mut out=Vec::new();
    while out.len()<rate as usize/2 {
        engine.render(&[],&mut l,&mut r);engine.end_block(block);out.extend_from_slice(&l);
    }
    out.truncate(rate as usize/2);out
}
fn rms(x:&[f32])->f64 {(x.iter().map(|x|f64::from(*x).powi(2)).sum::<f64>()/x.len() as f64).sqrt()}
#[test]
fn six_characters_sound_and_are_not_merely_level_variations() {
    let p=ModelFParams::default();let mut sounds=Vec::new();
    for mode in 0..6 {
        let mut c=p.controls();c.panel.character=mode;c.panel.character_shape=0.0;
        if mode==5 {c.supersaw=Some(0.6);c.panel.osc[1].on=false;}
        let x=render(&c,48000.0,128);let tail=&x[12000..];let energy=rms(tail);
        let peak=x.iter().map(|x|x.abs()).fold(0.0f32,f32::max);
        println!("Character {mode}: rms {energy:.6}, peak {peak:.6}");
        assert!(x.iter().all(|x|x.is_finite())&&energy>0.0001&&peak<4.0);
        if let Some(original)=sounds.first() {
            let _=original;assert!(energy>0.008 && energy<0.08,"reference level mismatch mode {mode}: {energy}");
        }
        sounds.push(tail.iter().map(|x|f64::from(*x)/energy).collect::<Vec<_>>());
    }
    for i in 0..6 {for j in 0..i {
        let difference=sounds[i].iter().zip(&sounds[j]).map(|(a,b)|(a-b).powi(2)).sum::<f64>()/sounds[i].len() as f64;
        assert!(difference>0.002,"{i} / {j} level-normalised difference too small: {difference}");
    }}
}
#[test]
fn preset_recall_keeps_global_tuning_and_recalls_character() {
    let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<ModelF>::new(p.clone() as Arc<dyn Params>,48000.0,ProcessMode::Offline);
    let set=|id:&str,v:f32| {let q=p.get(id).unwrap();assert!(ctx.automate(q.as_ptr(),q.preview_normalized(v)));};
    set("global_tuning",432.0);set("character",4.0);set("character_shape",0.7);
    let own=presets::Preset::capture(&p,"Acid Test".into(),"Bass".into());own.validate().unwrap();
    assert!(!own.values.contains_key("global_tuning"));
    set("global_tuning",528.0);set("character",2.0);
    let gui=TestGui(p.clone());own.apply(&p,&ParamSetter::new(&gui)).unwrap();
    assert_eq!(p.global_tuning.value(),528.0);assert_eq!(p.character.value(),4.0);
    assert!((p.character_shape.value()-0.7).abs()<0.001);assert!(!own.differs(&p));
    for sound in presets::factory() {sound.apply(&p,&ParamSetter::new(&gui)).unwrap();assert_eq!(p.character.value(),0.0);assert_eq!(p.global_tuning.value(),528.0);}
}
#[test]
fn old_session_defaults_and_new_state_roundtrip() {
    use nih_plug::wrapper::state::{PluginState,ParamValue};
    let mut state=PluginState{version:"0.2.3".into(),params:Default::default(),fields:Default::default()};
    ModelF::filter_state(&mut state);
    for (id,v) in [("character",0.0),("character_shape",0.0),("global_tuning",440.0)] {assert!(matches!(state.params.get(id),Some(ParamValue::F32(x)) if *x==v));}
    state.params.insert("character".into(),ParamValue::F32(5.0));state.params.insert("global_tuning".into(),ParamValue::F32(432.0));
    let encoded=serde_json::to_vec(&state).unwrap();let mut back:PluginState=serde_json::from_slice(&encoded).unwrap();ModelF::filter_state(&mut back);
    assert!(matches!(back.params.get("character"),Some(ParamValue::F32(x)) if *x==5.0));
    assert!(matches!(back.params.get("global_tuning"),Some(ParamValue::F32(x)) if *x==432.0));
}
#[test]
fn general_tuning_uses_frequency_ratio_for_both_oscillators() {
    let mut p=ModelFParams::default();
    p.global_tuning=FloatParam::new("Tuning",432.0,FloatRange::Linear{min:400.0,max:480.0});
    assert!((p.controls().global_cents - 1200.0*(432.0f64/440.0).log2()).abs()<1e-10);
    p.character=FloatParam::new("Character",5.0,FloatRange::Linear{min:0.0,max:5.0});
    let c=p.controls();assert!(c.supersaw.is_some()&&c.poly&&c.voices==8&&!c.panel.osc[1].on);
}
#[test]
fn shapes_and_block_boundaries_retain_finite_sound() {
    for mode in 1..6 {
        let mut c=ModelFParams::default().controls();c.panel.character=mode;
        c.panel.character_shape=0.8;c.panel.emphasis=0.95;c.panel.cutoff=0.65;
        let a=render(&c,44100.0,64);let b=render(&c,44100.0,127);
        assert!(a.iter().zip(&b).all(|(a,b)|(a-b).abs()<1e-6),"block invariance mode {mode}");
        assert!(a.iter().all(|x|x.is_finite()&&x.abs()<4.0));
    }
}
#[test]
fn compact_selector_and_modal_locks() {
    let p=ModelFParams::default();let normal=panel::View::default();let svg=panel::svg(&p,&normal);
    assert!(!svg.contains("160 FACTORY SOUNDS"));
    for label in panel::CHARACTER_NAMES {assert!(svg.contains(label));}
    assert!(svg.contains("GLOBAL TUNING")&&svg.contains("440.0 Hz"));
    for (i,a) in panel::CHARACTER_RECTS.iter().enumerate() {
        assert!(a.0>=340.0 && a.0+a.2<=770.0 && a.1+a.3<118.0);
        for b in &panel::CHARACTER_RECTS[..i] {assert!(a.0>=b.0+b.2||b.0>=a.0+a.2);}
    }
    for view in [panel::View{envelope:true,..Default::default()},panel::View{browser:true,..Default::default()}] {
        assert!(view.blocks_click(350.0,95.0));assert!(!panel::control_enabled_for_view(&p,&view,"global_tuning"));
    }
}

#[test]
fn tuning_changes_the_rendered_note_by_the_expected_ratio() {
    fn frequency(c:&Controls)->f64 {
        let x=render(c,48000.0,128);let mut crossings=Vec::new();
        for (i,w) in x.windows(2).enumerate().skip(12000) {
            if w[0]<0.0 && w[1]>=0.0 {crossings.push(i as f64+f64::from(-w[0]/(w[1]-w[0])));}
        }
        assert!(crossings.len()>10);
        (crossings.len()-1) as f64*48000.0/(crossings.last().unwrap()-crossings[0])
    }
    let mut c=ModelFParams::default().controls();c.lock=true;c.panel.osc[1].on=false;
    let base=frequency(&c);
    for tuning in panel::GLOBAL_TUNING_CHOICES {
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<ModelF>::new(p.clone() as Arc<dyn Params>,48000.0,ProcessMode::Offline);
        assert!(ctx.automate(p.global_tuning.as_ptr(),p.global_tuning.preview_normalized(tuning)));
        assert!((p.global_tuning.value()-tuning).abs()<0.051);
        c.global_cents=p.controls().global_cents;let tuned=frequency(&c);
        println!("Global tuning {tuning} Hz: {base} -> {tuned} Hz");
        assert!((tuned/base-f64::from(tuning)/440.0).abs()<0.0005);
    }
}
#[test]
fn character_switches_begin_continuously_and_survive_fast_changes() {
    for target in 1..6 {
        let mut c=ModelFParams::default().controls();let mut e=Engine::new();let mut reference=Engine::new();
        for engine in [&mut e,&mut reference] {engine.set(&c);engine.prepare(48000.0,42);engine.event(Event::Note{key:45,on:true});}
        let (mut l,mut r,mut a,mut b)=([0f32;128],[0f32;128],[0f32;128],[0f32;128]);
        for _ in 0..40 {e.render(&[],&mut l,&mut r);e.end_block(128);reference.render(&[],&mut a,&mut b);reference.end_block(128);}
        c.panel.character=target;e.set(&c);e.render(&[],&mut l,&mut r);reference.render(&[],&mut a,&mut b);
        assert!((l[0]-a[0]).abs()<0.001,"start discontinuity character {target}");
        for i in 0..60 {c.panel.character=(i%6) as u8;c.panel.character_shape=i as f64/60.0;e.set(&c);e.render(&[],&mut l,&mut r);e.end_block(128);assert!(l.iter().chain(&r).all(|x|x.is_finite()&&x.abs()<4.0));}
    }
}
