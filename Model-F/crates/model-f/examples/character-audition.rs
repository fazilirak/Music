// SPDX-License-Identifier: GPL-3.0-or-later
// Geliştirici: Fazıl Irak | Düzenleme: 26.10.2026
// Gerçek Engine'den seviye eşlemeli ses örnekleri ve tek-thread CPU matrisi.
use model_f::{engine::{Engine,Event},params::ModelFParams,panel};
use nih_plug::prelude::*;
use std::{time::Instant,io::Write};
fn wav(path:&std::path::Path,x:&[f32]) {
    let mut f=std::fs::File::create(path).unwrap();let size=(x.len()*2) as u32;
    for bytes in [b"RIFF".as_slice(),&(size+36).to_le_bytes(),b"WAVEfmt ",&16u32.to_le_bytes(),&1u16.to_le_bytes(),&2u16.to_le_bytes(),&48000u32.to_le_bytes(),&192000u32.to_le_bytes(),&4u16.to_le_bytes(),&16u16.to_le_bytes(),b"data",&size.to_le_bytes()] {f.write_all(bytes).unwrap();}
    for y in x {f.write_all(&((y.clamp(-1.0,1.0)*32767.0) as i16).to_le_bytes()).unwrap();}
}
fn main() {
    let out=std::path::PathBuf::from(std::env::args().nth(1).unwrap());std::fs::create_dir_all(&out).unwrap();
    let mut rows=Vec::new();
    for character in 0..6 {
        let p=ModelFParams{character:FloatParam::new("Character",character as f32,FloatRange::Linear{min:0.0,max:5.0}),..Default::default()};
        for (name,notes,super_mode) in [("Mono",1,false),("Poly",4,false),("SuperSaw",4,true)] {
            for rate in [44100.0,48000.0,96000.0] {for block in [64usize,256] {
                let mut c=p.controls();c.poly=notes>1||character==5;c.voices=8;
                if super_mode {c.supersaw=Some(0.5);c.panel.osc[1].on=false;}
                let mut e=Engine::new();e.set(&c);e.prepare(rate,42);
                for key in 45..45+notes {e.event(Event::Note{key,on:true});}
                let mut l=vec![0.0;block];let mut r=l.clone();
                // Başlangıç rampası ölçüm dışında; yarım saniye sürekli nota işlenir.
                for _ in 0..4 {e.render(&[],&mut l,&mut r);e.end_block(block);}
                let blocks=(rate as usize/2)/block;let start=Instant::now();let mut peak=0.0f32;
                for _ in 0..blocks {e.render(&[],&mut l,&mut r);e.end_block(block);for x in l.iter().chain(&r){assert!(x.is_finite());peak=peak.max(x.abs());}}
                let seconds=blocks as f64*block as f64/rate;let elapsed=start.elapsed().as_secs_f64();
                rows.push(serde_json::json!({"character":panel::CHARACTER_NAMES[character],"case":name,"notes":notes,"rate":rate,"block":block,"audio_seconds":seconds,"render_ms":elapsed*1000.0,"one_thread_audio_percent":elapsed/seconds*100.0,"peak":peak}));
            }}
        }
        let mut c=p.controls();c.panel.emphasis=0.65;c.panel.contour_amount=0.65;c.panel.filter_contour.decay=0.30;c.panel.loudness_contour.decay=0.25;
        let mut e=Engine::new();e.set(&c);e.prepare(48000.0,42);let(mut l,mut r)=([0.0;128],[0.0;128]);let mut audio=Vec::new();
        let notes=[45,45,57,48,45,52,55,43];
        for (ix,key) in notes.into_iter().enumerate() {
            e.event(Event::Note{key,on:true});
            for b in 0..112 {
                if b==72 {e.event(Event::Note{key,on:false});}
                c.panel.cutoff=0.3+ix as f64*0.055;e.set(&c);e.render(&[],&mut l,&mut r);e.end_block(128);
                for i in 0..128 {audio.extend([l[i],r[i]]);}
            }
        }
        let name=panel::CHARACTER_NAMES[character].replace(' ',"-");wav(&out.join(format!("character-{name}.wav")),&audio);
        let svg=panel::svg(&p,&panel::View{name:format!("{} — Character",panel::CHARACTER_NAMES[character]),..Default::default()});
        panel::render(&svg,panel::W,panel::H).unwrap().save_png(out.join(format!("panel-{name}.png"))).unwrap();
        println!("{} audition and performance scenarios complete",panel::CHARACTER_NAMES[character]);
    }
    std::fs::write(out.join("character-performance.json"),serde_json::to_vec_pretty(&rows).unwrap()).unwrap();
}
