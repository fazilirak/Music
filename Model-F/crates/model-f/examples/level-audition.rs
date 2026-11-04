// Fazıl Irak | Düzenleme: 02.11.2026
// Gerçek Engine üzerinden ham peak/RMS, düşük frekans enerjisi ve örnek WAV.
use model_f::{engine::{Engine,Event},params::ModelFParams,presets};
use nih_plug::{prelude::*,context::process::TestProcessContext};
use std::{sync::Arc,io::Write,time::Instant};
fn main() {
    let out=std::path::PathBuf::from(std::env::args().nth(1).expect("output directory"));
    std::fs::create_dir_all(&out).unwrap();
    let bank=presets::factory();let mut rows=Vec::new();
    let selected=[("Init",None), ("Bass",bank.iter().position(|p|p.category=="Bass")),
        ("Lead",bank.iter().position(|p|p.category=="Lead")),("Pad",bank.iter().position(|p|p.category=="Pad"))];
    for (name,index) in selected {
        let p=Arc::new(ModelFParams::default());
        let mut ctx=TestProcessContext::<model_f::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        if let Some(i)=index {for (id,param) in p.list(){
            if let Some(v)=bank[i].values.get(id){assert!(ctx.automate(param.as_ptr(),param.preview_normalized(*v)));}
        }}
        for character in 0..6 {
            assert!(ctx.automate(p.character.as_ptr(),p.character.preview_normalized(character as f32)));
            let mut c=p.controls();
            if std::env::args().nth(2).as_deref()==Some("legacy") {c.full_output=false;}
            if name=="Init" {c.panel.cutoff=0.85;c.panel.contour_amount=0.;c.panel.loudness_contour.sustain=0.8;}
            let mut e=Engine::new();e.set(&c);e.prepare(48000.,42);
            let notes:Vec<u8>=if name=="Pad"{vec![48,55,60,64]}else{vec![if name=="Bass"{45}else{60}]};
            for &key in &notes{e.event(Event::Note{key,on:true});}
            let (mut l,mut r)=([0f32;256],[0f32;256]);let mut audio=Vec::new();
            let(mut peak,mut energy,mut lowenergy,mut low)=(0f64,0f64,0f64,[0f64;2]);
            let start=Instant::now();
            for block in 0..375 {if block==300{for &key in &notes{e.event(Event::Note{key,on:false});}}
                e.render(&[],&mut l,&mut r);e.end_block(256);
                for i in 0..256 {for (ch,v) in [l[i],r[i]].into_iter().enumerate(){
                    assert!(v.is_finite());let x=f64::from(v);peak=peak.max(x.abs());energy+=x*x;
                    low[ch]+=0.02584*(x-low[ch]);lowenergy+=low[ch]*low[ch];audio.push(v);
                }}
            }
            let n=audio.len() as f64;let rms=(energy/n).sqrt();
            rows.push(serde_json::json!({"case":name,"character":character,"peak":peak,"peak_dbfs":20.*peak.max(1e-12).log10(),"rms_dbfs":20.*rms.max(1e-12).log10(),"low_energy_ratio":lowenergy/energy.max(1e-20),"render_ms":start.elapsed().as_secs_f64()*1000.}));
            if character==0 {let mut f=std::fs::File::create(out.join(format!("{name}.wav"))).unwrap();let bytes=(audio.len()*4) as u32;
                for b in [b"RIFF".as_slice(),&(bytes+36).to_le_bytes(),b"WAVEfmt ",&16u32.to_le_bytes(),&3u16.to_le_bytes(),&2u16.to_le_bytes(),&48000u32.to_le_bytes(),&384000u32.to_le_bytes(),&8u16.to_le_bytes(),&32u16.to_le_bytes(),b"data",&bytes.to_le_bytes()]{f.write_all(b).unwrap();}
                for x in audio{f.write_all(&x.to_le_bytes()).unwrap();}
            }
        }
    }
    std::fs::write(out.join("levels.json"),serde_json::to_vec_pretty(&rows).unwrap()).unwrap();
    println!("{} cases rendered",rows.len());
}
