//! Compact, label/value modulation editor. EG geometry and gestures share one transform.
use crate::{mseg, eg_editor::{self,State,Edits,Command},mod_view::View,modulators,params::ModelFParams,panel};
use nih_plug::prelude::*;
use keyboard_types::{Key,Modifiers};
use std::fmt::Write;
const GRAPH:(f64,f64,f64,f64)=(204.,218.,608.,205.);
const STEPS:(f64,f64,f64,f64)=(204.,486.,860.,137.);
const WAVES:[&str;8]=["SINE","TRIANGLE","SAW UP","SAW DOWN","SQUARE","PULSE","STAIRS","RANDOM"];
fn point(x:f64,y:f64)->(f64,f64) {(56.+(x-GRAPH.0)/GRAPH.2*1008.,244.+(y-GRAPH.1)/GRAPH.3*240.)}
fn map(v:&View<'_>,e:Edits)->Edits {e.into_iter().map(|(id,x)|(v.real_id(&id),x)).collect()}
pub(crate) fn activate(p:&ModelFParams,mut e:Edits)->Edits {
    if !e.is_empty() && p.mod_system.value()<0.5 {
        // Only a visibly enabled legacy Multi EG is transferred. The hidden AD
        // path must not turn on another target on the user's first edit.
        if p.mseg_amount.value()==0. || p.mseg_enable.value()<0.5 {e.push(("mod_system".into(),1.));return e;}
        let target=(p.mseg_target.value() as usize).saturating_sub(1).min(2);
        let page=[2,0,5][target];let v=View::bank(p,page);
        let mut migrated:Edits=p.list().into_iter().filter(|(id,_)|id.starts_with("mseg_")&&!matches!(*id,"mseg_attack"|"mseg_decay"|"mseg_target")).map(|(id,param)|(v.real_id(id),param.value())).collect();
        if target==1 {
            let copy:Edits=migrated.iter().map(|(id,value)|(id.replacen("mod_pitch_","mod_pitch2_",1),*value)).collect();migrated.extend(copy);
        }
        migrated.append(&mut e);e=migrated;e.push(("mod_system".into(),1.));
    } e
}
fn rows(s:&State)->Vec<(&'static str,&'static str,f64,f64)> {let _=s;vec![
    ("mseg_enable","Status",48.,222.),("mseg_amount","Depth",48.,246.),("mseg_duration","Time",48.,270.),("mseg_mode","Playback",48.,294.),
    ("mseg_speed","Speed",48.,318.),("mseg_fix_tempo","Fix Tempo",48.,342.),("mseg_gridx","X Grid",48.,366.),("mseg_gridy","Y Grid",48.,390.),("mseg_snap","Snap",48.,414.),
    ("lfo_status","Status",838.,300.),("lfo_freq","Freq",838.,324.),("lfo_sync","Sync",838.,348.),("lfo_phase","Phase",838.,372.),("lfo_delay","Delay",838.,396.),("lfo_fade","Fade",838.,420.),("lfo_depth","Depth",838.,444.),("mseg_brush","EG Brush",658.,459.),
    ("step_status","Status",48.,490.),("step_count","Steps",48.,514.),("step_freq","Freq",48.,538.),("step_sync","Sync",48.,562.),("step_smooth","Smooth",48.,586.),("step_depth","Depth",48.,610.),
]}
fn row_width(id:&str,x:f64)->f64 {if id=="mseg_brush" {154.}else if x>800. {226.}else{140.}}
fn cycle(id:&str)->bool {matches!(id,"mseg_enable"|"mseg_mode"|"mseg_speed"|"mseg_fix_tempo"|"mseg_gridx"|"mseg_gridy"|"mseg_snap"|"mseg_brush"|"lfo_status"|"lfo_sync"|"step_status"|"step_sync"|"step_retrigger"|"step_snap")}
fn step(v:&View<'_>,x:f64,y:f64,reset:bool)->Edits {
    let n=v.get("step_count").unwrap().value() as usize;
    let i=(((x-STEPS.0)/STEPS.2*n as f64).floor() as usize).min(n-1);
    let mut val=if reset {0.5}else{(1.-(y-STEPS.1)/STEPS.3).clamp(0.,1.) as f32};
    let divisions=[0.,10.,12.,24.][v.get("step_snap").unwrap().value() as usize];
    if divisions>0. {val=(val*divisions).round()/divisions;}
    vec![(v.real_id(&format!("step_{i}")),val)]
}
pub fn down(p:&ModelFParams,s:&mut State,x:f64,y:f64,right:bool,mods:Modifiers)->(Edits,Command) {
    if x>1000.&&y<174. {return(vec![],Command::Close)}
    if (180. ..208.).contains(&y) && (204. ..1064.).contains(&x) {s.page=(((x-204.)/(860./6.)) as usize).min(5);s.selected=0;s.drag=None;s.extra_drag=None;s.step_drag=false;return(vec![],Command::None)}
    let v=View::editing(p,s.page);
    if panel::in_rect(GRAPH,x,y) {let (x,y)=point(x,y);let (e,c)=s.down(&v,x,y,right,mods);return(activate(p,map(&v,e)),c)}
    if panel::in_rect(STEPS,x,y) {s.step_drag=!right;return(activate(p,step(&v,x,y,mods.contains(Modifiers::CONTROL))),Command::None)}
    if (144. ..170.).contains(&y) {
        if (694. ..772.).contains(&x) {
            let defaults=View::bank(p,s.page);
            let e=defaults.list().into_iter().filter(|(id,_)|id.starts_with("mseg_p")||id.starts_with("mseg_n")||matches!(*id,"mseg_start"|"mseg_curve"))
                .map(|(id,param)|(defaults.real_id(id),param.default_plain_value())).collect();
            return(activate(p,e),Command::None)
        }
        if (788. ..866.).contains(&x) {return(vec![],Command::Save)}
        if (882. ..960.).contains(&x) {return(vec![],Command::Load)}
    }
    if (444. ..468.).contains(&y) {
        if (204. ..370.).contains(&x) {let id=format!("mseg_n{}_wave",s.selected);let wave=(v.get(&id).unwrap().value() as usize+if right {8}else{1})%9;return(activate(p,map(&v,vec![(id,wave as f32),("mseg_brush".into(),wave.max(1) as f32)])),Command::None)}
        if (388. ..518.).contains(&x) {let id=format!("mseg_n{}_depth",s.selected);s.extra_drag=Some((id.clone(),y,v.get(&id).unwrap().unmodulated_normalized_value()));return(vec![],Command::None)}
        if (536. ..640.).contains(&x) {if mods.intersects(Modifiers::CONTROL|Modifiers::ALT) {return(activate(p,map(&v,vec![("mseg_curve".into(),0.)])),Command::None)}s.extra_drag=Some(("mseg_curve".into(),y,v.mseg_curve.unmodulated_normalized_value()));return(vec![],Command::None)}
    }
    if (216. ..286.).contains(&y) && (838. ..1064.).contains(&x) {let par=v.get("lfo_wave").unwrap();return(activate(p,map(&v,vec![("lfo_wave".into(),(par.value()+if right {7.}else{1.})%8.)])),Command::None)}
    if (638. ..658.).contains(&y) {
        if (204. ..440.).contains(&x) {let par=v.get("step_retrigger").unwrap();return(activate(p,map(&v,vec![("step_retrigger".into(),1.-par.value())])),Command::None)}
        if (460. ..650.).contains(&x) {let par=v.get("step_snap").unwrap();return(activate(p,map(&v,vec![("step_snap".into(),(par.value()+1.)%4.)])),Command::None)}
        if (690. ..812.).contains(&x) {return(activate(p,(0..128).map(|i|(v.real_id(&format!("step_{i}")),0.5)).collect()),Command::None)}
    }
    for (id,_,a,b) in rows(s) {let width=row_width(id,a);if panel::in_rect((a,b-16.,width,22.),x,y) {
        if (id=="mseg_duration"&&v.mseg_speed.value()>0.5)||(id=="mseg_fix_tempo"&&v.mseg_speed.value()<0.5) {return(vec![],Command::None)}
        let param=v.get(id).unwrap();
        if mods.intersects(Modifiers::CONTROL|Modifiers::ALT) {return(activate(p,map(&v,vec![(id.into(),param.default_plain_value())])),Command::None)}
        if cycle(id) {let lo=param.preview_plain(0.);let hi=param.preview_plain(1.);let mut n=param.value()+if right {-1.}else{1.};if n>hi {n=lo}if n<lo {n=hi}return(activate(p,map(&v,vec![(id.into(),n)])),Command::None)}
        s.extra_drag=Some((id.into(),y,param.unmodulated_normalized_value()));return(vec![],Command::None)
    }}
    (vec![],Command::None)
}
pub fn motion(p:&ModelFParams,s:&mut State,x:f64,y:f64,mods:Modifiers)->Edits {
    let v=View::editing(p,s.page);
    if s.step_drag {return activate(p,step(&v,x,y,mods.contains(Modifiers::CONTROL)))}
    if let Some((id,start,base))=&s.extra_drag {let param=v.get(id).unwrap();let n=base+((start-y)*if mods.contains(Modifiers::SHIFT){0.0005}else{0.005}) as f32;return activate(p,map(&v,vec![(id.clone(),param.preview_plain(n.clamp(0.,1.)))]))}
    let (x,y)=point(x,y);activate(p,map(&v,s.motion(&v,x,y,mods)))
}
pub fn key(p:&ModelFParams,s:&State,key:&Key,mods:Modifiers)->Edits {let v=View::editing(p,s.page);activate(p,map(&v,s.key(&v,key,mods)))}
pub fn wheel(p:&ModelFParams,s:&State,x:f64,y:f64,delta:f32,mods:Modifiers)->Edits {
    let v=View::editing(p,s.page);
    for (id,_,a,b) in rows(s) {if panel::in_rect((a,b-16.,row_width(id,a),22.),x,y) {let par=v.get(id).unwrap();let n=if cycle(id){par.value()+delta.signum()}else{par.value()+delta*(par.preview_plain(1.)-par.preview_plain(0.))*if mods.contains(Modifiers::SHIFT){0.001}else{0.01}};return activate(p,map(&v,vec![(id.into(),n.clamp(par.preview_plain(0.),par.preview_plain(1.)))]))}}
    vec![]
}
fn display(id:&str,v:f32)->String {match id {
    "mseg_duration"|"lfo_delay"|"lfo_fade"=>format!("{v:.2} s"),
    "lfo_freq"|"step_freq"=>format!("{v:.2} Hz"),
    "lfo_phase"=>format!("{:.0}°",v*360.),
    "mseg_enable"=>if v>0.5 {"ON"}else{"OFF"}.into(),"mseg_mode"=>["ONCE","LOOP","HOLD"][v as usize].into(),
    "mseg_speed"=>mseg::SPEEDS[v as usize].into(),"mseg_fix_tempo"|"mseg_snap"=>if v>0.5 {"ON"}else{"OFF"}.into(),
    "mseg_gridx"|"mseg_gridy"=>format!("{}",2_usize.pow(v as u32)),
    "mseg_brush"=>mseg::WAVES[v as usize].into(),
    "lfo_status"=>["OFF","ON","SHOT"][v as usize].into(),"step_status"=>["OFF","ON","SHOT","STEP"][v as usize].into(),
    "lfo_sync"|"step_sync"=>modulators::SYNC[v as usize].into(),"step_count"=>format!("{v:.0}"),
    "mseg_amount"|"lfo_depth"|"step_depth"|"step_smooth"=>format!("{:.1}%",v*100.),
    _=>format!("{v:.2}")}}
fn row(out:&mut String,x:f64,y:f64,w:f64,label:&str,value:&str) {
    let _=write!(out,r##"<path d="M{x} {}h{w}" stroke="#343d40"/>"##,y+6.);
    panel::text(out,x,y,11,"#899598",label,"start");panel::text(out,x+w,y,11,"#d9e1dc",value,"end");
}
// Status indicates an enabled source, including a source with zero depth.
pub fn target_active(p:&ModelFParams,page:usize)->bool {
    let v=View::editing(p,page);
    v.mseg_enable.value()>0.5 || v.get("lfo_status").unwrap().value()>0.5 || v.get("step_status").unwrap().value()>0.5
}
pub fn any_active(p:&ModelFParams)->bool {(0..6).any(|page|target_active(p,page))}
pub fn draw(out:&mut String,p:&ModelFParams,s:&State) {
    let v=View::editing(p,s.page);let c=mseg::Config::from_view(&v);let shape=c.shape();
    out.push_str(r##"<rect x="28" y="132" width="1064" height="542" rx="8" fill="#1b2123" stroke="#596265"/>"##);
    panel::text(out,48.,163.,16,"#e2e5de","MODULATORS","start");
    for (x,label) in [(694.,"RESET EG"),(788.,"SAVE EG"),(882.,"LOAD EG")] {panel::text(out,x,162.,11,"#a8b0ae",label,"start")}
    panel::text(out,1068.,162.,11,"#ed6c46","CLOSE ×","end");
    for (i,label) in modulators::LABELS.iter().enumerate() {
        let x=204.+i as f64*(860./6.);
        if i!=s.page && target_active(p,i) {
            let _=write!(out,r##"<rect x="{x}" y="180" width="139" height="28" rx="4" fill="#e5e7df" stroke="#e5e7df"/>"##);
            panel::text(out,x+69.5,198.,11,"#17191a",label,"middle");
        } else {panel::button(out,x,180.,139.,28.,label,i==s.page)}
    }
    panel::text(out,48.,196.,13,"#ed6c46","MULTI EG","start");panel::text(out,48.,464.,13,"#ed6c46","STEP","start");
    for (id,label,x,y) in rows(s) {let value=v.get(id).unwrap().value();let shown=if id=="mseg_duration"&&v.mseg_speed.value()>0.5 {"CLOCK".into()}else{display(id,value)};row(out,x,y,row_width(id,x),label,&shown);}
    let (gx,gy,gw,gh)=GRAPH;
    let _=write!(out,r##"<rect x="{gx}" y="{gy}" width="{gw}" height="{gh}" fill="#151b1e" stroke="#41494b"/>"##);
    for i in 1..2_usize.pow(v.mseg_gridx.value() as u32) {let x=gx+gw*i as f64/2_f64.powi(v.mseg_gridx.value() as i32);let _=write!(out,r##"<path d="M{x} {gy}v{gh}" stroke="#30393b"/>"##);}
    for i in 1..2_usize.pow(v.mseg_gridy.value() as u32) {let y=gy+gh*i as f64/2_f64.powi(v.mseg_gridy.value() as i32);let _=write!(out,r##"<path d="M{gx} {y}h{gw}" stroke="#30393b"/>"##);}
    let mut path=String::new();for i in 0..=608 {let t=i as f64/608.;let _=write!(path,"{} {:.2} {:.2} ",if i==0 {"M"}else{"L"},gx+t*gw,gy+(1.-shape.value(t))*gh);}
    let _=write!(out,r##"<path d="{path}" fill="none" stroke="{}" stroke-width="2"/>"##,if c.enabled {"#ed6c46"}else{"#667174"});
    for n in &shape.nodes[..shape.count] {let x=gx+n.x*gw;let y=gy+(1.-n.y)*gh;let _=write!(out,r##"<circle cx="{x}" cy="{y}" r="4" fill="#1b2123" stroke="{}"/>"##,if n.id==s.selected {"#fff"}else{"#ed6c46"});}
    for incoming in [true,false] {if let Some((x,y,_))=eg_editor::curve_handle(&shape,s.selected,incoming) {let x=gx+x*gw;let y=gy+(1.-y)*gh;let _=write!(out,r##"<rect x="{}" y="{}" width="7" height="7" fill="#c8d0cb"/>"##,x-3.5,y-3.5);}}
    row(out,204.,459.,166.,"Node",mseg::WAVES[v.get(&format!("mseg_n{}_wave",s.selected)).unwrap().value() as usize]);
    row(out,388.,459.,130.,"Node depth",&format!("{:.0}%",v.get(&format!("mseg_n{}_depth",s.selected)).unwrap().value()*100.));
    row(out,536.,459.,104.,"Curve",&format!("{:.2}",v.mseg_curve.value()));
    let wave=v.get("lfo_wave").unwrap().value() as usize;let mut path=String::new();for i in 0..=220 {let t=i as f64/220.;let value=modulators::oscillator(wave,t+f64::from(v.get("lfo_phase").unwrap().value()));let _=write!(path,"{} {:.1} {:.1} ",if i==0 {"M"}else{"L"},840.+t*220.,250.-value*30.);}
    let _=write!(out,r##"<rect x="838" y="216" width="226" height="70" fill="#151b1e" stroke="#41494b"/><path d="{path}" stroke="#a8c2c9" fill="none"/>"##);
    panel::text(out,846.,228.,9,"#ed6c46","LFO","start");panel::text(out,1058.,280.,9,"#899598",WAVES[wave],"end");
    let n=v.get("step_count").unwrap().value() as usize;let (x,y,w,h)=STEPS;let _=write!(out,r##"<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#151b1e" stroke="#41494b"/>"##);
    for i in 0..n {let val=f64::from(v.get(&format!("step_{i}")).unwrap().value());let a=x+i as f64*w/n as f64;let height=val*h;let _=write!(out,r##"<rect x="{a}" y="{}" width="{}" height="{height}" fill="#667881" stroke="#293338"/>"##,y+h-height,w/n as f64);}
    row(out,204.,652.,236.,"Step → EG",if v.get("step_retrigger").unwrap().value()>0.5 {"ON"}else{"OFF"});
    row(out,460.,652.,190.,"Step snap",["OFF","10","12","24"][v.get("step_snap").unwrap().value() as usize]);
    panel::text(out,690.,652.,10,"#899598","RESET STEPS","start");
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Arc;
    use nih_plug::context::process::TestProcessContext;
    fn apply(p:&ModelFParams,ctx:&TestProcessContext<crate::ModelF>,edits:Edits) {for (id,value) in edits {let par=p.get(&id).unwrap();assert!(ctx.automate(par.as_ptr(),par.preview_normalized(value)));}}
    #[test]
    fn activity_follows_source_status_instead_of_system_switch_or_depth() {
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        apply(&p,&ctx,vec![("mod_system".into(),1.)]);
        assert!(!any_active(&p));
        for page in 0..6 {for source in ["mseg_enable","lfo_status","step_status"] {
            let id=View::bank(&p,page).real_id(source);
            apply(&p,&ctx,vec![(id.clone(),1.)]);
            assert!(any_active(&p));
            for other in 0..6 {assert_eq!(target_active(&p,other),other==page);}
            apply(&p,&ctx,vec![(id,0.)]);assert!(!any_active(&p));
        }}
        apply(&p,&ctx,vec![("mod_system".into(),0.),("mseg_target".into(),2.),("mseg_amount".into(),0.5)]);
        assert!(!any_active(&p));
        apply(&p,&ctx,vec![("mseg_enable".into(),1.)]);
        assert!(any_active(&p));assert!(target_active(&p,0));assert!(target_active(&p,1));assert!(!target_active(&p,2));
    }
    #[test]
    fn sub_foundation_indicator_matches_all_visible_statuses() {
        let preset=crate::presets::factory().into_iter().find(|p|p.name=="Sub Foundation").unwrap();
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        apply(&p,&ctx,preset.values.into_iter().filter(|(id,_)|p.get(id).is_some()).collect());
        assert!(p.mod_system.value()<0.5);assert!(p.mseg_amount.value().abs()>0.0005);
        for page in 0..6 {
            let v=View::editing(&p,page);
            assert_eq!(v.mseg_enable.value(),0.);assert_eq!(v.get("lfo_status").unwrap().value(),0.);assert_eq!(v.get("step_status").unwrap().value(),0.);
            assert!(!target_active(&p,page));
        }
        assert!(!any_active(&p));
    }
    #[test]
    fn first_source_toggle_after_preset_reload_does_not_enable_other_targets() {
        let preset=crate::presets::factory().into_iter().find(|p|p.name=="Sub Foundation").unwrap();
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        let controls=[("mseg_enable",120.,216.),("lfo_status",960.,298.),("step_status",120.,484.)];
        for page in 0..6 {for (source,x,y) in controls {for _reload in 0..2 {
            let loaded=p.list().into_iter().filter(|(id,_)|*id!="global_tuning").map(|(id,par)| {
                let value=preset.values.get(id).copied().unwrap_or_else(||if matches!(id,"output_voicing"|"preamp_path") {0.}else{par.default_plain_value()});
                (id.into(),value)
            }).collect();apply(&p,&ctx,loaded);
            assert!(p.mod_system.value()<0.5);assert!(!any_active(&p));
            let mut state=State::default();state.page=page;
            for right in [false,true,false] {
                let (e,_)=down(&p,&mut state,x,y,right,Modifiers::empty());apply(&p,&ctx,e);
                for other in 0..6 {let view=View::editing(&p,other);for (id,_,_) in controls {
                    let expected=if other==page && id==source && !right {1.}else{0.};
                    assert_eq!(view.get(id).unwrap().value(),expected,"page {page}, source {source}: {other}/{id}");
                }}
            }
        }}}
    }
    #[test]
    fn new_and_reset_banks_start_with_simple_adsr_drawing() {
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        for page in 0..6 {
            let shape=mseg::Config::from_view(&View::bank(&p,page)).shape();
            assert_eq!(shape.count,5);
            let n=&shape.nodes[..shape.count];assert_eq!(n[0].y,0.);assert_eq!(n[1].y,1.);assert!((n[2].y-0.65).abs()<0.001);assert_eq!(n[2].y,n[3].y);assert_eq!(n[4].y,0.);
            let id=View::bank(&p,page).real_id("mseg_p5_on");apply(&p,&ctx,vec![(id.clone(),1.)]);
            let mut state=State::default();state.page=page;
            let (e,_)=down(&p,&mut state,720.,156.,false,Modifiers::empty());apply(&p,&ctx,e);
            let reset=mseg::Config::from_view(&View::bank(&p,page)).shape();assert_eq!(reset.count,5);assert_eq!(View::bank(&p,page).get("mseg_p5_on").unwrap().value(),0.);
        }
    }
    #[test]
    fn tabs_preserve_independent_eg_lfo_and_step_values() {
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);let mut s=State::default();
        for page in 0..6 {
            let (e,_)=down(&p,&mut s,210.+page as f64*(860./6.),190.,false,Modifiers::empty());assert!(e.is_empty());assert_eq!(s.page,page);
            let (e,_)=down(&p,&mut s,120.,216.,false,Modifiers::empty());apply(&p,&ctx,e);
            let (e,_)=down(&p,&mut s,960.,298.,false,Modifiers::empty());apply(&p,&ctx,e);
            let (e,_)=down(&p,&mut s,120.,484.,false,Modifiers::empty());apply(&p,&ctx,e);
            let v=View::bank(&p,page);assert_eq!(v.mseg_enable.value(),1.);assert_eq!(v.get("lfo_status").unwrap().value(),1.);assert_eq!(v.get("step_status").unwrap().value(),1.);
        }
        let configs=modulators::Config::from_params(&p);assert!(configs.enabled);assert!(configs.targets.iter().all(|d|d.eg.enabled&&d.lfo_status==1&&d.step_status==1));
        s.page=2;let (e,_)=down(&p,&mut s,215.,550.,false,Modifiers::empty());apply(&p,&ctx,e);assert!(View::bank(&p,2).get("step_0").unwrap().value()!=0.5);assert_eq!(View::bank(&p,0).get("step_0").unwrap().value(),0.5);
    }
    #[test]
    fn panel_draws_all_three_sections_without_ad_controls_at_two_scales() {
        let p=ModelFParams::default();for page in 0..6 {let mut v=panel::View {envelope:true,..Default::default()};v.env.page=page;let svg=panel::svg(&p,&v);assert!(svg.contains("MODULATORS"));assert!(!svg.contains("AD ENVELOPE"));assert!(svg.contains("Step → EG"));assert!(panel::render(&svg,1120,760).is_ok());assert!(panel::render(&svg,1680,1140).is_ok());}
    }
    #[test]
    fn first_edit_migrates_legacy_pitch_to_both_oscillators() {
        let p=Arc::new(ModelFParams::default());let ctx=TestProcessContext::<crate::ModelF>::new(p.clone() as Arc<dyn Params>,48000.,ProcessMode::Offline);
        for (id,value) in [("mseg_target",2.),("mseg_enable",1.),("mseg_amount",0.6),("mseg_p0_y",0.8)] {let param=p.get(id).unwrap();assert!(ctx.automate(param.as_ptr(),param.preview_normalized(value)));}
        let e=activate(&p,vec![("mod_cutoff_step_status".into(),1.)]);apply(&p,&ctx,e);
        for page in [0,1] {let view=View::bank(&p,page);assert_eq!(view.mseg_enable.value(),1.);assert!((view.mseg_amount.value()-0.6).abs()<0.001);assert!((view.mseg_p0_y.value()-0.8).abs()<0.001);}
        assert_eq!(View::bank(&p,2).get("step_status").unwrap().value(),1.);
    }
}
