// Geliştirici: Fazıl Irak | Son düzenleme: 24.10.2026
// Aynı tek iş parçacıklı ses akışının süre ve bit-hash karşılaştırması; DAW zamanlama garantisi değildir.
use model_f::{engine::{Engine, Event}, params::ModelFParams};
use std::time::Instant;
fn main() {
    let mut rows = Vec::new();
    for (name, notes, super_mode, target) in [("mono-idle",0,false,0),("mono",1,false,0),("mono-filter",1,false,1),("poly",4,false,0),("supersaw",4,true,0)] {
        let mut controls = ModelFParams::default().controls();
        controls.poly = notes > 1;
        controls.voices = 4;
        if super_mode { controls.supersaw=Some(0.5); controls.panel.osc[1].on=false; }
        if target > 0 { controls.mseg.enabled=true; controls.mseg.mode=1; controls.mseg.target=target; controls.mseg.amount=0.3; }
        let mut engine=Engine::new(); engine.set(&controls); engine.prepare(48000.,42);
        for key in 48..48+notes { engine.event(Event::Note{key,on:true}); }
        let (mut l,mut r)=([0f32;256],[0f32;256]);
        let mut hash=14695981039346656037u64;
        let start=Instant::now();
        for block in 0..375 {
            if block==188 { for key in 48..48+notes {engine.event(Event::Note{key,on:false});} }
            engine.render(&[],&mut l,&mut r); engine.end_block(256);
            for x in l.iter().chain(r.iter()) { assert!(x.is_finite()); hash^=u64::from(x.to_bits()); hash=hash.wrapping_mul(1099511628211); }
        }
        let ms=start.elapsed().as_secs_f64()*1000.;
        println!("{name}: {ms:.3} ms, hash {hash:016x}");
        rows.push(serde_json::json!({"case":name,"ms":ms,"hash":format!("{hash:016x}"),"sample_rate":48000,"samples":96000,"workers":0}));
    }
    std::fs::write(std::env::args().nth(1).expect("report path"),serde_json::to_vec_pretty(&rows).unwrap()).unwrap();
}
