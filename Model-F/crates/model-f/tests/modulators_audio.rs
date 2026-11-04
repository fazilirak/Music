//! Exercise each of the fifteen routes through the real instrument render path.
use model_f::engine::{Controls,Engine,Event};
fn render(target:Option<(usize,usize)>,poly:bool)->Vec<[f32;2]> {
    let mut c=Controls::default();c.poly=poly;c.voices=4;c.panel.cutoff=0.2;c.panel.emphasis=0.35;c.panel.osc[1].on=true;c.panel.osc[1].volume=0.6;
    if let Some((target,source))=target {
        c.modulators.enabled=true;let d=&mut c.modulators.targets[target];
        if source==0 {d.eg.enabled=true;d.eg.amount=0.4;d.eg.duration=0.2;d.eg.mode=1;}
        if source==1 {d.lfo_status=1;d.lfo_freq=7.;d.lfo_depth=0.3;}
        if source==2 {d.step_status=1;d.step_count=2;d.step_sync=0;d.step_freq=12.;d.step_depth=0.3;d.steps[0]=0.15;d.steps[1]=0.85;}
    }
    let mut e=Engine::new();e.set(&c);e.prepare(48000.,42);e.event(Event::Note {key:48,on:true});
    let (mut l,mut r)=([0.;128],[0.;128]);let mut out=Vec::new();
    for _ in 0..64 {e.render(&[],&mut l,&mut r);e.end_block(128);out.extend(l.iter().zip(&r).map(|(&a,&b)|[a,b]));}
    out
}
#[test]
fn each_eg_lfo_and_step_route_changes_real_mono_and_poly_audio() {
    for poly in [false,true] {
        let dry=render(None,poly);
        for target in 0..6 {for source in 0..3 {
            let wet=render(Some((target,source)),poly);assert!(wet.iter().flatten().all(|v|v.is_finite()));
            let difference: f64=wet.iter().zip(&dry).flat_map(|(a,b)|a.iter().zip(b).map(|(x,y)|f64::from((x-y).abs()))).sum();
            assert!(difference>0.01,"target {target} source {source} poly {poly}: route had no audible effect");
            if target==4 {let stereo:f64=wet.iter().map(|x|f64::from((x[0]-x[1]).abs())).sum();assert!(stereo>0.01);}
        }}
    }
}
