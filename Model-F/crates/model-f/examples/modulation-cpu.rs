use model_f::{engine::{Controls,Engine,Event},output_preamp};
use std::{time::Instant,path::PathBuf};
fn main() {
    let path=PathBuf::from(std::env::args().nth(1).expect("JSON output"));let mut rows=Vec::new();
    for (mode,notes,saw) in [("Mono",1,false),("Poly4",4,false),("SuperSaw8",8,true)] {
        for rate in [48000.,96000.] {for stage in 0..4 {
            let mut c=Controls::default();c.poly=notes>1;c.voices=8;
            if saw {c.supersaw=Some(0.4);c.panel.osc[1].on=false;}
            c.modulators.enabled=stage>0;
            for (i,d) in c.modulators.targets.iter_mut().enumerate() {d.eg.enabled=true;d.eg.amount=0.15;d.eg.mode=1;d.eg.duration=0.4;d.lfo_status=1;d.lfo_freq=0.5+i as f64*0.1;d.lfo_depth=0.1;d.step_status=1;d.step_count=16;d.step_sync=0;d.step_freq=2.;d.step_smooth=0.2;d.step_depth=0.15;for (n,v) in d.steps.iter_mut().enumerate() {*v=0.25+(n%5) as f64*0.1;}}
            c.preamp=output_preamp::Config {available:stage>1,mic:stage==3,gain_db:18.,trim_db:-6.,..Default::default()};
            let mut e=Engine::new();e.set(&c);e.prepare(rate,42);for key in 48..48+notes {e.event(Event::Note {key,on:true});}
            let(mut l,mut r)=([0.;128],[0.;128]);for _ in 0..8 {e.render(&[],&mut l,&mut r);e.end_block(128);}
            let blocks=(rate*0.35/128.) as usize;let mut hash=14695981039346656037_u64;let mut peak=0_f32;let start=Instant::now();
            for _ in 0..blocks {e.render(&[],&mut l,&mut r);e.end_block(128);for x in l.iter().chain(&r) {assert!(x.is_finite());hash^=u64::from(x.to_bits());hash=hash.wrapping_mul(1099511628211);peak=peak.max(x.abs());}}
            let elapsed=start.elapsed().as_secs_f64();let seconds=(blocks*128) as f64/rate;
            rows.push(serde_json::json!({"mode":mode,"rate":rate,"stage":(["Dry","18 modulators","18 modulators + LINE","18 modulators + MIC"][stage]),"audio_seconds":seconds,"render_ms":elapsed*1000.,"one_core_percent":elapsed/seconds*100.,"sample_hash":format!("{hash:016x}"),"peak":peak}));
            println!("{mode} {rate:.0} stage {stage}: {:.2}%",elapsed/seconds*100.);
        }}
    }
    std::fs::write(path,serde_json::to_vec_pretty(&rows).unwrap()).unwrap();
}
