//! Five independent per-voice modulation destinations. Fixed storage on the audio thread.
use crate::{mseg, params::ModelFParams};
pub const LABELS: [&str;6] = ["OSC 1 PITCH", "OSC 2 PITCH", "CUTOFF", "RESONANCE", "PAN", "AMP"];
pub const BEATS: [f64;16] = [0.,1./12.,0.125,0.1875,0.25,0.375,0.5,0.75,1.,1.5,2.,3.,4.,8.,16.,32.];
pub const SYNC: [&str;16] = ["OFF","1/8T","1/8","1/8D","1/4","1/4D","1/2","1/2D","1","1D","2","2D","4","8","16","32"];
#[derive(Debug,Clone,Copy,PartialEq)]
pub struct Destination {
    pub eg: mseg::Config, pub speed: usize, pub fix: bool,
    pub lfo_status: usize, pub lfo_wave: usize, pub lfo_freq: f64, pub lfo_sync: usize,
    pub lfo_phase: f64, pub lfo_delay: f64, pub lfo_fade: f64, pub lfo_depth: f64,
    pub step_status: usize, pub step_count: usize, pub step_freq: f64, pub step_sync: usize,
    pub step_smooth: f64, pub step_depth: f64, pub step_retrigger: bool, pub steps: [f64;128],
}
impl Default for Destination { fn default()->Self {Self {eg:mseg::Config::default(),speed:0,fix:false,lfo_status:0,lfo_wave:0,lfo_freq:1.,lfo_sync:0,lfo_phase:0.,lfo_delay:0.,lfo_fade:0.,lfo_depth:0.,step_status:0,step_count:16,step_freq:1.,step_sync:6,step_smooth:0.,step_depth:0.,step_retrigger:false,steps:[0.5;128]}} }
#[derive(Debug,Clone,Copy,PartialEq)]
pub struct Config {pub enabled:bool,pub targets:[Destination;6],pub bpm:f64,pub beat:Option<f64>}
impl Default for Config {fn default()->Self {Self {enabled:false,targets:[Destination::default();6],bpm:120.,beat:None}}}
impl Config {
    pub fn tempo(&mut self,bpm:Option<f64>,beat:Option<f64>) {
        if let Some(bpm)=bpm.filter(|v| v.is_finite() && *v>0.) {self.bpm=bpm.clamp(20.,400.);}
        self.beat=beat.filter(|v|v.is_finite());
        for d in &mut self.targets {if d.speed>0 {d.eg.duration=60./if d.fix {120.}else{self.bpm}*8./2_f64.powi(d.speed as i32-1);}}
    }
}
#[derive(Debug,Clone)]
struct Running {eg:mseg::Envelope,lfo:f64,step:f64,age:f64,gate:bool,previous_step:usize,smooth:f64,started:bool,lfo_increment:f64,step_increment:f64,smooth_coeff:f64}
impl Default for Running {fn default()->Self {Self {eg:Default::default(),lfo:0.,step:0.,age:0.,gate:false,previous_step:0,smooth:0.5,started:false,lfo_increment:0.,step_increment:0.,smooth_coeff:1.}}}
#[derive(Debug,Clone)]
pub struct Runtime {config:Config,rate:f64,run:[Running;6],offset:f64}
impl Default for Runtime {fn default()->Self {Self {config:Default::default(),rate:48000.,run:std::array::from_fn(|_|Default::default()),offset:0.}}}
#[derive(Debug,Clone,Copy)]
pub struct Output {pub gain:f64,pub pan:f64}
impl Runtime {
    pub fn note_on(&mut self) {for r in &mut self.run {r.gate=false;}}
    pub fn configure(&mut self,c:Config,rate:f64) {
        self.rate=rate.max(1.);
        self.offset=0.;
        for (r,d) in self.run.iter_mut().zip(c.targets) {
            r.eg.configure(d.eg,self.rate);
            if c.enabled && !self.config.enabled { *r=Running::default();r.eg.configure(d.eg,self.rate); }
            let frequency=|hz:f64,sync:usize|if sync==0 {hz}else{c.bpm/60./BEATS[sync.min(15)]};
            r.lfo_increment=frequency(d.lfo_freq,d.lfo_sync)/self.rate;
            r.step_increment=frequency(d.step_freq,d.step_sync)/self.rate;
            r.smooth_coeff=if d.step_smooth==0. {1.}else{1.-libm::exp(-1./(self.rate*(0.001+d.step_smooth*0.5)))};
            if let Some(beat)=c.beat {
                if d.lfo_sync>0 && d.lfo_status==1 {r.lfo=beat/BEATS[d.lfo_sync]+d.lfo_phase;}
                if d.step_sync>0 && d.step_status==1 {r.step=(beat/BEATS[d.step_sync]).rem_euclid(1.);}
            }
        }
        self.config=c;
    }
    pub fn reset(&mut self) {for r in &mut self.run {*r=Running::default();} for (r,d) in self.run.iter_mut().zip(self.config.targets) {r.eg.configure(d.eg,self.rate);}}
    pub fn tick(&mut self,gate:bool,j:&mut model_f_core::voice::Jacks)->Output {
        if !self.config.enabled {return Output {gain:1.,pan:0.};}
        let mut values=[0.;6];let mut amp=1.;
        for (i,(r,d)) in self.run.iter_mut().zip(self.config.targets.iter()).enumerate() {
            let onset=gate&&!r.gate;
            if onset {r.age=0.;r.lfo=d.lfo_phase;r.eg.reset();
                if d.lfo_sync>0 && d.lfo_status==1 {if let Some(beat)=self.config.beat {r.lfo=(beat+self.offset*self.config.bpm/60.)/BEATS[d.lfo_sync]+d.lfo_phase;}}
                if d.step_status==3 {r.step=if r.started {(r.step+1./d.step_count as f64).fract()}else{0.};}
                else {r.step=if d.step_sync>0 && d.step_status==1 {self.config.beat.map_or(0.,|beat|((beat+self.offset*self.config.bpm/60.)/BEATS[d.step_sync]).rem_euclid(1.))}else{0.};}
                r.previous_step=(r.step*d.step_count as f64) as usize;
                r.smooth=d.steps[r.previous_step.min(127)];
                r.started=true;
            }
            r.gate=gate;
            let index=((r.step*d.step_count as f64) as usize).min(d.step_count-1);
            if d.step_status!=0 && index!=r.previous_step && d.step_retrigger {r.eg.reset();}
            r.previous_step=index;
            let eg=if d.eg.enabled {r.eg.tick(gate)}else{0.};
            let fade=if d.lfo_fade<=0. {1.}else{((r.age-d.lfo_delay)/d.lfo_fade).clamp(0.,1.)};
            let intensity=if d.lfo_status!=0 && r.age>=d.lfo_delay && (d.lfo_status!=2 || r.lfo-d.lfo_phase<1.) {fade}else{0.};
            let lfo=if intensity==0. || d.lfo_depth==0. {0.}else{oscillator(d.lfo_wave,r.lfo)*intensity*d.lfo_depth};
            if r.age>=d.lfo_delay {r.lfo+=r.lfo_increment;}
            let coeff=r.smooth_coeff;
            r.smooth+=(d.steps[index]-r.smooth)*coeff;
            let step=if d.step_status!=0 {(r.smooth*2.-1.)*d.step_depth}else{0.};
            if d.step_status==1 {r.step=(r.step+r.step_increment).fract();}
            if d.step_status==2 {r.step=(r.step+r.step_increment).min(1.-f64::EPSILON);}
            if i==5 {
                let depth=d.eg.amount.abs().min(1.);
                let e=if d.eg.amount<0. {1.-eg}else{eg};
                if d.eg.enabled {amp*=1.-depth+depth*e;}
                // Bipolar LFO and step values attenuate around unity; depth zero is bypass.
                if d.lfo_status!=0 {amp*=(1.-d.lfo_depth.abs()*intensity*0.5+lfo*0.5).clamp(0.,1.);}
                if d.step_status!=0 {amp*= (1.-d.step_depth.abs()+if d.step_depth>=0. {r.smooth}else{1.-r.smooth}*d.step_depth.abs()).clamp(0.,1.);}
            } else {values[i]=eg*d.eg.amount+lfo+step;}
            r.age+=1./self.rate;
        }
        j.detune[0]+=values[0]*4800.;j.detune[1]+=values[1]*4800.;
        j.cutoff+=values[2]*4.;j.resonance+=values[3];
        self.offset+=1./self.rate;
        Output {gain:amp,pan:values[4].clamp(-1.,1.)}
    }
}
pub fn oscillator(wave:usize,phase:f64)->f64 {let t=phase.rem_euclid(1.);match wave {
    0=>libm::sin(std::f64::consts::TAU*t),1=>1.-4.*(t-0.5).abs(),2=>2.*t-1.,3=>1.-2.*t,
    4=>if t<0.5 {1.}else{-1.},5=>if t<0.25 {1.}else{-1.},6=>2.*(t*4.).floor()/3.-1.,
    _=>{let x=(phase.floor() as i64).wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);((x as u64>>32) as f64/u32::MAX as f64)*2.-1.}
}}
pub fn pan(p:f64)->(f64,f64) {((1.-p.max(0.)).sqrt(),(1.+p.min(0.)).sqrt())}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn disabled_bank_is_exact_bypass() {
        let mut r=Runtime::default();let mut j=Default::default();
        let out=r.tick(true,&mut j);assert_eq!(out.gain,1.);assert_eq!(out.pan,0.);assert_eq!(j.resonance,0.);assert_eq!(j.cutoff,0.);assert_eq!(j.detune,[0.;3]);
    }
    #[test]
    fn five_destinations_run_together_without_sharing_state() {
        for rate in [44100.,48000.,96000.] {
            let mut c=Config {enabled:true,..Default::default()};
            for d in &mut c.targets {d.lfo_status=1;d.lfo_wave=4;d.lfo_depth=0.25;d.step_sync=0;}
            let mut r=Runtime::default();r.configure(c,rate);let mut j=Default::default();let o=r.tick(true,&mut j);
            assert_eq!(j.detune[0],1200.);assert_eq!(j.cutoff,1.);assert_eq!(j.resonance,0.25);assert_eq!(o.pan,0.25);assert!((0. ..=1.).contains(&o.gain));
            c.targets[0].lfo_status=0;r.configure(c,rate);let mut j=Default::default();r.tick(true,&mut j);assert_eq!(j.detune[0],0.);assert_eq!(j.cutoff,1.);
        }
    }
    #[test]
    fn lfo_delay_fade_and_one_shot_are_note_relative() {
        let mut c=Config {enabled:true,..Default::default()};let d=&mut c.targets[0];d.lfo_status=2;d.lfo_wave=4;d.lfo_depth=1.;d.lfo_freq=10.;d.lfo_delay=0.01;d.lfo_fade=0.01;
        let mut r=Runtime::default();r.configure(c,1000.);let mut values=Vec::new();for _ in 0..150 {let mut j=Default::default();r.tick(true,&mut j);values.push(j.detune[0]);}
        assert!(values[..10].iter().all(|x|*x==0.));assert!(values[16]>0.&&values[16]<4800.);assert_eq!(values[140],0.);
    }
    #[test]
    fn step_count_smoothing_and_retrigger_work_at_cycle_boundaries() {
        let mut c=Config {enabled:true,..Default::default()};let d=&mut c.targets[0];d.step_status=1;d.step_count=2;d.step_freq=10.;d.step_sync=0;d.step_depth=1.;d.steps[0]=0.;d.steps[1]=1.;d.eg.enabled=true;d.eg.amount=0.;d.eg.duration=0.2;d.step_retrigger=true;
        let mut r=Runtime::default();r.configure(c,1000.);let mut values=Vec::new();for _ in 0..120 {let mut j=Default::default();r.tick(true,&mut j);values.push(j.detune[0]);}
        assert_eq!(values[10],-4800.);assert_eq!(values[70],4800.);assert_eq!(values[110],-4800.);assert!(r.run[0].eg.clone().tick(true).is_finite());
    }
    #[test]
    fn tempo_uses_whole_pattern_and_preserves_free_eg_seconds() {
        let mut c=Config::default();c.targets[0].speed=1;c.targets[1].speed=6;c.targets[1].fix=true;c.targets[2].eg.duration=3.25;c.tempo(Some(60.),Some(8.));
        assert_eq!(c.targets[0].eg.duration,8.);assert_eq!(c.targets[1].eg.duration,0.125);assert_eq!(c.targets[2].eg.duration,3.25);
    }
    #[test]
    fn amp_off_sources_are_unity_and_pan_is_bounded() {
        let mut c=Config {enabled:true,..Default::default()};c.targets[5].lfo_depth=1.;c.targets[5].step_depth=1.;let mut r=Runtime::default();r.configure(c,48000.);assert_eq!(r.tick(true,&mut Default::default()).gain,1.);
        assert_eq!(pan(-1.),(1.,0.));assert_eq!(pan(1.),(0.,1.));assert_eq!(pan(0.),(1.,1.));
    }
    #[test]
    fn note_step_mode_advances_even_without_a_gate_release() {
        let mut c=Config {enabled:true,..Default::default()};let d=&mut c.targets[0];d.step_status=3;d.step_count=2;d.step_depth=1.;d.steps[0]=0.;d.steps[1]=1.;
        let mut r=Runtime::default();r.configure(c,48000.);
        for expected in [-4800.,4800.,-4800.] {r.note_on();let mut j=Default::default();r.tick(true,&mut j);assert_eq!(j.detune[0],expected);}
    }
}

impl Config {pub fn from_params(p:&ModelFParams)->Self {Self {enabled:p.mod_system.value()>0.5,bpm:f64::from(p.tempo.value()),beat:None,targets:[
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,0)),speed:p.mod_pitch_mseg_speed.value() as usize,fix:p.mod_pitch_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_pitch_lfo_status.value() as usize,
lfo_wave:p.mod_pitch_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_pitch_lfo_freq.value()),
lfo_sync:p.mod_pitch_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_pitch_lfo_phase.value()),
lfo_delay:f64::from(p.mod_pitch_lfo_delay.value()),
lfo_fade:f64::from(p.mod_pitch_lfo_fade.value()),
lfo_depth:f64::from(p.mod_pitch_lfo_depth.value()),
step_status:p.mod_pitch_step_status.value() as usize,
step_count:p.mod_pitch_step_count.value() as usize,
step_freq:f64::from(p.mod_pitch_step_freq.value()),
step_sync:p.mod_pitch_step_sync.value() as usize,
step_smooth:f64::from(p.mod_pitch_step_smooth.value()),
step_depth:f64::from(p.mod_pitch_step_depth.value()),
step_retrigger:p.mod_pitch_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_pitch_step_0.value()),f64::from(p.mod_pitch_step_1.value()),f64::from(p.mod_pitch_step_2.value()),f64::from(p.mod_pitch_step_3.value()),f64::from(p.mod_pitch_step_4.value()),f64::from(p.mod_pitch_step_5.value()),f64::from(p.mod_pitch_step_6.value()),f64::from(p.mod_pitch_step_7.value()),f64::from(p.mod_pitch_step_8.value()),f64::from(p.mod_pitch_step_9.value()),f64::from(p.mod_pitch_step_10.value()),f64::from(p.mod_pitch_step_11.value()),f64::from(p.mod_pitch_step_12.value()),f64::from(p.mod_pitch_step_13.value()),f64::from(p.mod_pitch_step_14.value()),f64::from(p.mod_pitch_step_15.value()),f64::from(p.mod_pitch_step_16.value()),f64::from(p.mod_pitch_step_17.value()),f64::from(p.mod_pitch_step_18.value()),f64::from(p.mod_pitch_step_19.value()),f64::from(p.mod_pitch_step_20.value()),f64::from(p.mod_pitch_step_21.value()),f64::from(p.mod_pitch_step_22.value()),f64::from(p.mod_pitch_step_23.value()),f64::from(p.mod_pitch_step_24.value()),f64::from(p.mod_pitch_step_25.value()),f64::from(p.mod_pitch_step_26.value()),f64::from(p.mod_pitch_step_27.value()),f64::from(p.mod_pitch_step_28.value()),f64::from(p.mod_pitch_step_29.value()),f64::from(p.mod_pitch_step_30.value()),f64::from(p.mod_pitch_step_31.value()),f64::from(p.mod_pitch_step_32.value()),f64::from(p.mod_pitch_step_33.value()),f64::from(p.mod_pitch_step_34.value()),f64::from(p.mod_pitch_step_35.value()),f64::from(p.mod_pitch_step_36.value()),f64::from(p.mod_pitch_step_37.value()),f64::from(p.mod_pitch_step_38.value()),f64::from(p.mod_pitch_step_39.value()),f64::from(p.mod_pitch_step_40.value()),f64::from(p.mod_pitch_step_41.value()),f64::from(p.mod_pitch_step_42.value()),f64::from(p.mod_pitch_step_43.value()),f64::from(p.mod_pitch_step_44.value()),f64::from(p.mod_pitch_step_45.value()),f64::from(p.mod_pitch_step_46.value()),f64::from(p.mod_pitch_step_47.value()),f64::from(p.mod_pitch_step_48.value()),f64::from(p.mod_pitch_step_49.value()),f64::from(p.mod_pitch_step_50.value()),f64::from(p.mod_pitch_step_51.value()),f64::from(p.mod_pitch_step_52.value()),f64::from(p.mod_pitch_step_53.value()),f64::from(p.mod_pitch_step_54.value()),f64::from(p.mod_pitch_step_55.value()),f64::from(p.mod_pitch_step_56.value()),f64::from(p.mod_pitch_step_57.value()),f64::from(p.mod_pitch_step_58.value()),f64::from(p.mod_pitch_step_59.value()),f64::from(p.mod_pitch_step_60.value()),f64::from(p.mod_pitch_step_61.value()),f64::from(p.mod_pitch_step_62.value()),f64::from(p.mod_pitch_step_63.value()),f64::from(p.mod_pitch_step_64.value()),f64::from(p.mod_pitch_step_65.value()),f64::from(p.mod_pitch_step_66.value()),f64::from(p.mod_pitch_step_67.value()),f64::from(p.mod_pitch_step_68.value()),f64::from(p.mod_pitch_step_69.value()),f64::from(p.mod_pitch_step_70.value()),f64::from(p.mod_pitch_step_71.value()),f64::from(p.mod_pitch_step_72.value()),f64::from(p.mod_pitch_step_73.value()),f64::from(p.mod_pitch_step_74.value()),f64::from(p.mod_pitch_step_75.value()),f64::from(p.mod_pitch_step_76.value()),f64::from(p.mod_pitch_step_77.value()),f64::from(p.mod_pitch_step_78.value()),f64::from(p.mod_pitch_step_79.value()),f64::from(p.mod_pitch_step_80.value()),f64::from(p.mod_pitch_step_81.value()),f64::from(p.mod_pitch_step_82.value()),f64::from(p.mod_pitch_step_83.value()),f64::from(p.mod_pitch_step_84.value()),f64::from(p.mod_pitch_step_85.value()),f64::from(p.mod_pitch_step_86.value()),f64::from(p.mod_pitch_step_87.value()),f64::from(p.mod_pitch_step_88.value()),f64::from(p.mod_pitch_step_89.value()),f64::from(p.mod_pitch_step_90.value()),f64::from(p.mod_pitch_step_91.value()),f64::from(p.mod_pitch_step_92.value()),f64::from(p.mod_pitch_step_93.value()),f64::from(p.mod_pitch_step_94.value()),f64::from(p.mod_pitch_step_95.value()),f64::from(p.mod_pitch_step_96.value()),f64::from(p.mod_pitch_step_97.value()),f64::from(p.mod_pitch_step_98.value()),f64::from(p.mod_pitch_step_99.value()),f64::from(p.mod_pitch_step_100.value()),f64::from(p.mod_pitch_step_101.value()),f64::from(p.mod_pitch_step_102.value()),f64::from(p.mod_pitch_step_103.value()),f64::from(p.mod_pitch_step_104.value()),f64::from(p.mod_pitch_step_105.value()),f64::from(p.mod_pitch_step_106.value()),f64::from(p.mod_pitch_step_107.value()),f64::from(p.mod_pitch_step_108.value()),f64::from(p.mod_pitch_step_109.value()),f64::from(p.mod_pitch_step_110.value()),f64::from(p.mod_pitch_step_111.value()),f64::from(p.mod_pitch_step_112.value()),f64::from(p.mod_pitch_step_113.value()),f64::from(p.mod_pitch_step_114.value()),f64::from(p.mod_pitch_step_115.value()),f64::from(p.mod_pitch_step_116.value()),f64::from(p.mod_pitch_step_117.value()),f64::from(p.mod_pitch_step_118.value()),f64::from(p.mod_pitch_step_119.value()),f64::from(p.mod_pitch_step_120.value()),f64::from(p.mod_pitch_step_121.value()),f64::from(p.mod_pitch_step_122.value()),f64::from(p.mod_pitch_step_123.value()),f64::from(p.mod_pitch_step_124.value()),f64::from(p.mod_pitch_step_125.value()),f64::from(p.mod_pitch_step_126.value()),f64::from(p.mod_pitch_step_127.value())]},
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,1)),speed:p.mod_pitch2_mseg_speed.value() as usize,fix:p.mod_pitch2_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_pitch2_lfo_status.value() as usize,
lfo_wave:p.mod_pitch2_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_pitch2_lfo_freq.value()),
lfo_sync:p.mod_pitch2_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_pitch2_lfo_phase.value()),
lfo_delay:f64::from(p.mod_pitch2_lfo_delay.value()),
lfo_fade:f64::from(p.mod_pitch2_lfo_fade.value()),
lfo_depth:f64::from(p.mod_pitch2_lfo_depth.value()),
step_status:p.mod_pitch2_step_status.value() as usize,
step_count:p.mod_pitch2_step_count.value() as usize,
step_freq:f64::from(p.mod_pitch2_step_freq.value()),
step_sync:p.mod_pitch2_step_sync.value() as usize,
step_smooth:f64::from(p.mod_pitch2_step_smooth.value()),
step_depth:f64::from(p.mod_pitch2_step_depth.value()),
step_retrigger:p.mod_pitch2_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_pitch2_step_0.value()),f64::from(p.mod_pitch2_step_1.value()),f64::from(p.mod_pitch2_step_2.value()),f64::from(p.mod_pitch2_step_3.value()),f64::from(p.mod_pitch2_step_4.value()),f64::from(p.mod_pitch2_step_5.value()),f64::from(p.mod_pitch2_step_6.value()),f64::from(p.mod_pitch2_step_7.value()),f64::from(p.mod_pitch2_step_8.value()),f64::from(p.mod_pitch2_step_9.value()),f64::from(p.mod_pitch2_step_10.value()),f64::from(p.mod_pitch2_step_11.value()),f64::from(p.mod_pitch2_step_12.value()),f64::from(p.mod_pitch2_step_13.value()),f64::from(p.mod_pitch2_step_14.value()),f64::from(p.mod_pitch2_step_15.value()),f64::from(p.mod_pitch2_step_16.value()),f64::from(p.mod_pitch2_step_17.value()),f64::from(p.mod_pitch2_step_18.value()),f64::from(p.mod_pitch2_step_19.value()),f64::from(p.mod_pitch2_step_20.value()),f64::from(p.mod_pitch2_step_21.value()),f64::from(p.mod_pitch2_step_22.value()),f64::from(p.mod_pitch2_step_23.value()),f64::from(p.mod_pitch2_step_24.value()),f64::from(p.mod_pitch2_step_25.value()),f64::from(p.mod_pitch2_step_26.value()),f64::from(p.mod_pitch2_step_27.value()),f64::from(p.mod_pitch2_step_28.value()),f64::from(p.mod_pitch2_step_29.value()),f64::from(p.mod_pitch2_step_30.value()),f64::from(p.mod_pitch2_step_31.value()),f64::from(p.mod_pitch2_step_32.value()),f64::from(p.mod_pitch2_step_33.value()),f64::from(p.mod_pitch2_step_34.value()),f64::from(p.mod_pitch2_step_35.value()),f64::from(p.mod_pitch2_step_36.value()),f64::from(p.mod_pitch2_step_37.value()),f64::from(p.mod_pitch2_step_38.value()),f64::from(p.mod_pitch2_step_39.value()),f64::from(p.mod_pitch2_step_40.value()),f64::from(p.mod_pitch2_step_41.value()),f64::from(p.mod_pitch2_step_42.value()),f64::from(p.mod_pitch2_step_43.value()),f64::from(p.mod_pitch2_step_44.value()),f64::from(p.mod_pitch2_step_45.value()),f64::from(p.mod_pitch2_step_46.value()),f64::from(p.mod_pitch2_step_47.value()),f64::from(p.mod_pitch2_step_48.value()),f64::from(p.mod_pitch2_step_49.value()),f64::from(p.mod_pitch2_step_50.value()),f64::from(p.mod_pitch2_step_51.value()),f64::from(p.mod_pitch2_step_52.value()),f64::from(p.mod_pitch2_step_53.value()),f64::from(p.mod_pitch2_step_54.value()),f64::from(p.mod_pitch2_step_55.value()),f64::from(p.mod_pitch2_step_56.value()),f64::from(p.mod_pitch2_step_57.value()),f64::from(p.mod_pitch2_step_58.value()),f64::from(p.mod_pitch2_step_59.value()),f64::from(p.mod_pitch2_step_60.value()),f64::from(p.mod_pitch2_step_61.value()),f64::from(p.mod_pitch2_step_62.value()),f64::from(p.mod_pitch2_step_63.value()),f64::from(p.mod_pitch2_step_64.value()),f64::from(p.mod_pitch2_step_65.value()),f64::from(p.mod_pitch2_step_66.value()),f64::from(p.mod_pitch2_step_67.value()),f64::from(p.mod_pitch2_step_68.value()),f64::from(p.mod_pitch2_step_69.value()),f64::from(p.mod_pitch2_step_70.value()),f64::from(p.mod_pitch2_step_71.value()),f64::from(p.mod_pitch2_step_72.value()),f64::from(p.mod_pitch2_step_73.value()),f64::from(p.mod_pitch2_step_74.value()),f64::from(p.mod_pitch2_step_75.value()),f64::from(p.mod_pitch2_step_76.value()),f64::from(p.mod_pitch2_step_77.value()),f64::from(p.mod_pitch2_step_78.value()),f64::from(p.mod_pitch2_step_79.value()),f64::from(p.mod_pitch2_step_80.value()),f64::from(p.mod_pitch2_step_81.value()),f64::from(p.mod_pitch2_step_82.value()),f64::from(p.mod_pitch2_step_83.value()),f64::from(p.mod_pitch2_step_84.value()),f64::from(p.mod_pitch2_step_85.value()),f64::from(p.mod_pitch2_step_86.value()),f64::from(p.mod_pitch2_step_87.value()),f64::from(p.mod_pitch2_step_88.value()),f64::from(p.mod_pitch2_step_89.value()),f64::from(p.mod_pitch2_step_90.value()),f64::from(p.mod_pitch2_step_91.value()),f64::from(p.mod_pitch2_step_92.value()),f64::from(p.mod_pitch2_step_93.value()),f64::from(p.mod_pitch2_step_94.value()),f64::from(p.mod_pitch2_step_95.value()),f64::from(p.mod_pitch2_step_96.value()),f64::from(p.mod_pitch2_step_97.value()),f64::from(p.mod_pitch2_step_98.value()),f64::from(p.mod_pitch2_step_99.value()),f64::from(p.mod_pitch2_step_100.value()),f64::from(p.mod_pitch2_step_101.value()),f64::from(p.mod_pitch2_step_102.value()),f64::from(p.mod_pitch2_step_103.value()),f64::from(p.mod_pitch2_step_104.value()),f64::from(p.mod_pitch2_step_105.value()),f64::from(p.mod_pitch2_step_106.value()),f64::from(p.mod_pitch2_step_107.value()),f64::from(p.mod_pitch2_step_108.value()),f64::from(p.mod_pitch2_step_109.value()),f64::from(p.mod_pitch2_step_110.value()),f64::from(p.mod_pitch2_step_111.value()),f64::from(p.mod_pitch2_step_112.value()),f64::from(p.mod_pitch2_step_113.value()),f64::from(p.mod_pitch2_step_114.value()),f64::from(p.mod_pitch2_step_115.value()),f64::from(p.mod_pitch2_step_116.value()),f64::from(p.mod_pitch2_step_117.value()),f64::from(p.mod_pitch2_step_118.value()),f64::from(p.mod_pitch2_step_119.value()),f64::from(p.mod_pitch2_step_120.value()),f64::from(p.mod_pitch2_step_121.value()),f64::from(p.mod_pitch2_step_122.value()),f64::from(p.mod_pitch2_step_123.value()),f64::from(p.mod_pitch2_step_124.value()),f64::from(p.mod_pitch2_step_125.value()),f64::from(p.mod_pitch2_step_126.value()),f64::from(p.mod_pitch2_step_127.value())]},
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,2)),speed:p.mod_cutoff_mseg_speed.value() as usize,fix:p.mod_cutoff_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_cutoff_lfo_status.value() as usize,
lfo_wave:p.mod_cutoff_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_cutoff_lfo_freq.value()),
lfo_sync:p.mod_cutoff_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_cutoff_lfo_phase.value()),
lfo_delay:f64::from(p.mod_cutoff_lfo_delay.value()),
lfo_fade:f64::from(p.mod_cutoff_lfo_fade.value()),
lfo_depth:f64::from(p.mod_cutoff_lfo_depth.value()),
step_status:p.mod_cutoff_step_status.value() as usize,
step_count:p.mod_cutoff_step_count.value() as usize,
step_freq:f64::from(p.mod_cutoff_step_freq.value()),
step_sync:p.mod_cutoff_step_sync.value() as usize,
step_smooth:f64::from(p.mod_cutoff_step_smooth.value()),
step_depth:f64::from(p.mod_cutoff_step_depth.value()),
step_retrigger:p.mod_cutoff_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_cutoff_step_0.value()),f64::from(p.mod_cutoff_step_1.value()),f64::from(p.mod_cutoff_step_2.value()),f64::from(p.mod_cutoff_step_3.value()),f64::from(p.mod_cutoff_step_4.value()),f64::from(p.mod_cutoff_step_5.value()),f64::from(p.mod_cutoff_step_6.value()),f64::from(p.mod_cutoff_step_7.value()),f64::from(p.mod_cutoff_step_8.value()),f64::from(p.mod_cutoff_step_9.value()),f64::from(p.mod_cutoff_step_10.value()),f64::from(p.mod_cutoff_step_11.value()),f64::from(p.mod_cutoff_step_12.value()),f64::from(p.mod_cutoff_step_13.value()),f64::from(p.mod_cutoff_step_14.value()),f64::from(p.mod_cutoff_step_15.value()),f64::from(p.mod_cutoff_step_16.value()),f64::from(p.mod_cutoff_step_17.value()),f64::from(p.mod_cutoff_step_18.value()),f64::from(p.mod_cutoff_step_19.value()),f64::from(p.mod_cutoff_step_20.value()),f64::from(p.mod_cutoff_step_21.value()),f64::from(p.mod_cutoff_step_22.value()),f64::from(p.mod_cutoff_step_23.value()),f64::from(p.mod_cutoff_step_24.value()),f64::from(p.mod_cutoff_step_25.value()),f64::from(p.mod_cutoff_step_26.value()),f64::from(p.mod_cutoff_step_27.value()),f64::from(p.mod_cutoff_step_28.value()),f64::from(p.mod_cutoff_step_29.value()),f64::from(p.mod_cutoff_step_30.value()),f64::from(p.mod_cutoff_step_31.value()),f64::from(p.mod_cutoff_step_32.value()),f64::from(p.mod_cutoff_step_33.value()),f64::from(p.mod_cutoff_step_34.value()),f64::from(p.mod_cutoff_step_35.value()),f64::from(p.mod_cutoff_step_36.value()),f64::from(p.mod_cutoff_step_37.value()),f64::from(p.mod_cutoff_step_38.value()),f64::from(p.mod_cutoff_step_39.value()),f64::from(p.mod_cutoff_step_40.value()),f64::from(p.mod_cutoff_step_41.value()),f64::from(p.mod_cutoff_step_42.value()),f64::from(p.mod_cutoff_step_43.value()),f64::from(p.mod_cutoff_step_44.value()),f64::from(p.mod_cutoff_step_45.value()),f64::from(p.mod_cutoff_step_46.value()),f64::from(p.mod_cutoff_step_47.value()),f64::from(p.mod_cutoff_step_48.value()),f64::from(p.mod_cutoff_step_49.value()),f64::from(p.mod_cutoff_step_50.value()),f64::from(p.mod_cutoff_step_51.value()),f64::from(p.mod_cutoff_step_52.value()),f64::from(p.mod_cutoff_step_53.value()),f64::from(p.mod_cutoff_step_54.value()),f64::from(p.mod_cutoff_step_55.value()),f64::from(p.mod_cutoff_step_56.value()),f64::from(p.mod_cutoff_step_57.value()),f64::from(p.mod_cutoff_step_58.value()),f64::from(p.mod_cutoff_step_59.value()),f64::from(p.mod_cutoff_step_60.value()),f64::from(p.mod_cutoff_step_61.value()),f64::from(p.mod_cutoff_step_62.value()),f64::from(p.mod_cutoff_step_63.value()),f64::from(p.mod_cutoff_step_64.value()),f64::from(p.mod_cutoff_step_65.value()),f64::from(p.mod_cutoff_step_66.value()),f64::from(p.mod_cutoff_step_67.value()),f64::from(p.mod_cutoff_step_68.value()),f64::from(p.mod_cutoff_step_69.value()),f64::from(p.mod_cutoff_step_70.value()),f64::from(p.mod_cutoff_step_71.value()),f64::from(p.mod_cutoff_step_72.value()),f64::from(p.mod_cutoff_step_73.value()),f64::from(p.mod_cutoff_step_74.value()),f64::from(p.mod_cutoff_step_75.value()),f64::from(p.mod_cutoff_step_76.value()),f64::from(p.mod_cutoff_step_77.value()),f64::from(p.mod_cutoff_step_78.value()),f64::from(p.mod_cutoff_step_79.value()),f64::from(p.mod_cutoff_step_80.value()),f64::from(p.mod_cutoff_step_81.value()),f64::from(p.mod_cutoff_step_82.value()),f64::from(p.mod_cutoff_step_83.value()),f64::from(p.mod_cutoff_step_84.value()),f64::from(p.mod_cutoff_step_85.value()),f64::from(p.mod_cutoff_step_86.value()),f64::from(p.mod_cutoff_step_87.value()),f64::from(p.mod_cutoff_step_88.value()),f64::from(p.mod_cutoff_step_89.value()),f64::from(p.mod_cutoff_step_90.value()),f64::from(p.mod_cutoff_step_91.value()),f64::from(p.mod_cutoff_step_92.value()),f64::from(p.mod_cutoff_step_93.value()),f64::from(p.mod_cutoff_step_94.value()),f64::from(p.mod_cutoff_step_95.value()),f64::from(p.mod_cutoff_step_96.value()),f64::from(p.mod_cutoff_step_97.value()),f64::from(p.mod_cutoff_step_98.value()),f64::from(p.mod_cutoff_step_99.value()),f64::from(p.mod_cutoff_step_100.value()),f64::from(p.mod_cutoff_step_101.value()),f64::from(p.mod_cutoff_step_102.value()),f64::from(p.mod_cutoff_step_103.value()),f64::from(p.mod_cutoff_step_104.value()),f64::from(p.mod_cutoff_step_105.value()),f64::from(p.mod_cutoff_step_106.value()),f64::from(p.mod_cutoff_step_107.value()),f64::from(p.mod_cutoff_step_108.value()),f64::from(p.mod_cutoff_step_109.value()),f64::from(p.mod_cutoff_step_110.value()),f64::from(p.mod_cutoff_step_111.value()),f64::from(p.mod_cutoff_step_112.value()),f64::from(p.mod_cutoff_step_113.value()),f64::from(p.mod_cutoff_step_114.value()),f64::from(p.mod_cutoff_step_115.value()),f64::from(p.mod_cutoff_step_116.value()),f64::from(p.mod_cutoff_step_117.value()),f64::from(p.mod_cutoff_step_118.value()),f64::from(p.mod_cutoff_step_119.value()),f64::from(p.mod_cutoff_step_120.value()),f64::from(p.mod_cutoff_step_121.value()),f64::from(p.mod_cutoff_step_122.value()),f64::from(p.mod_cutoff_step_123.value()),f64::from(p.mod_cutoff_step_124.value()),f64::from(p.mod_cutoff_step_125.value()),f64::from(p.mod_cutoff_step_126.value()),f64::from(p.mod_cutoff_step_127.value())]},
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,3)),speed:p.mod_resonance_mseg_speed.value() as usize,fix:p.mod_resonance_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_resonance_lfo_status.value() as usize,
lfo_wave:p.mod_resonance_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_resonance_lfo_freq.value()),
lfo_sync:p.mod_resonance_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_resonance_lfo_phase.value()),
lfo_delay:f64::from(p.mod_resonance_lfo_delay.value()),
lfo_fade:f64::from(p.mod_resonance_lfo_fade.value()),
lfo_depth:f64::from(p.mod_resonance_lfo_depth.value()),
step_status:p.mod_resonance_step_status.value() as usize,
step_count:p.mod_resonance_step_count.value() as usize,
step_freq:f64::from(p.mod_resonance_step_freq.value()),
step_sync:p.mod_resonance_step_sync.value() as usize,
step_smooth:f64::from(p.mod_resonance_step_smooth.value()),
step_depth:f64::from(p.mod_resonance_step_depth.value()),
step_retrigger:p.mod_resonance_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_resonance_step_0.value()),f64::from(p.mod_resonance_step_1.value()),f64::from(p.mod_resonance_step_2.value()),f64::from(p.mod_resonance_step_3.value()),f64::from(p.mod_resonance_step_4.value()),f64::from(p.mod_resonance_step_5.value()),f64::from(p.mod_resonance_step_6.value()),f64::from(p.mod_resonance_step_7.value()),f64::from(p.mod_resonance_step_8.value()),f64::from(p.mod_resonance_step_9.value()),f64::from(p.mod_resonance_step_10.value()),f64::from(p.mod_resonance_step_11.value()),f64::from(p.mod_resonance_step_12.value()),f64::from(p.mod_resonance_step_13.value()),f64::from(p.mod_resonance_step_14.value()),f64::from(p.mod_resonance_step_15.value()),f64::from(p.mod_resonance_step_16.value()),f64::from(p.mod_resonance_step_17.value()),f64::from(p.mod_resonance_step_18.value()),f64::from(p.mod_resonance_step_19.value()),f64::from(p.mod_resonance_step_20.value()),f64::from(p.mod_resonance_step_21.value()),f64::from(p.mod_resonance_step_22.value()),f64::from(p.mod_resonance_step_23.value()),f64::from(p.mod_resonance_step_24.value()),f64::from(p.mod_resonance_step_25.value()),f64::from(p.mod_resonance_step_26.value()),f64::from(p.mod_resonance_step_27.value()),f64::from(p.mod_resonance_step_28.value()),f64::from(p.mod_resonance_step_29.value()),f64::from(p.mod_resonance_step_30.value()),f64::from(p.mod_resonance_step_31.value()),f64::from(p.mod_resonance_step_32.value()),f64::from(p.mod_resonance_step_33.value()),f64::from(p.mod_resonance_step_34.value()),f64::from(p.mod_resonance_step_35.value()),f64::from(p.mod_resonance_step_36.value()),f64::from(p.mod_resonance_step_37.value()),f64::from(p.mod_resonance_step_38.value()),f64::from(p.mod_resonance_step_39.value()),f64::from(p.mod_resonance_step_40.value()),f64::from(p.mod_resonance_step_41.value()),f64::from(p.mod_resonance_step_42.value()),f64::from(p.mod_resonance_step_43.value()),f64::from(p.mod_resonance_step_44.value()),f64::from(p.mod_resonance_step_45.value()),f64::from(p.mod_resonance_step_46.value()),f64::from(p.mod_resonance_step_47.value()),f64::from(p.mod_resonance_step_48.value()),f64::from(p.mod_resonance_step_49.value()),f64::from(p.mod_resonance_step_50.value()),f64::from(p.mod_resonance_step_51.value()),f64::from(p.mod_resonance_step_52.value()),f64::from(p.mod_resonance_step_53.value()),f64::from(p.mod_resonance_step_54.value()),f64::from(p.mod_resonance_step_55.value()),f64::from(p.mod_resonance_step_56.value()),f64::from(p.mod_resonance_step_57.value()),f64::from(p.mod_resonance_step_58.value()),f64::from(p.mod_resonance_step_59.value()),f64::from(p.mod_resonance_step_60.value()),f64::from(p.mod_resonance_step_61.value()),f64::from(p.mod_resonance_step_62.value()),f64::from(p.mod_resonance_step_63.value()),f64::from(p.mod_resonance_step_64.value()),f64::from(p.mod_resonance_step_65.value()),f64::from(p.mod_resonance_step_66.value()),f64::from(p.mod_resonance_step_67.value()),f64::from(p.mod_resonance_step_68.value()),f64::from(p.mod_resonance_step_69.value()),f64::from(p.mod_resonance_step_70.value()),f64::from(p.mod_resonance_step_71.value()),f64::from(p.mod_resonance_step_72.value()),f64::from(p.mod_resonance_step_73.value()),f64::from(p.mod_resonance_step_74.value()),f64::from(p.mod_resonance_step_75.value()),f64::from(p.mod_resonance_step_76.value()),f64::from(p.mod_resonance_step_77.value()),f64::from(p.mod_resonance_step_78.value()),f64::from(p.mod_resonance_step_79.value()),f64::from(p.mod_resonance_step_80.value()),f64::from(p.mod_resonance_step_81.value()),f64::from(p.mod_resonance_step_82.value()),f64::from(p.mod_resonance_step_83.value()),f64::from(p.mod_resonance_step_84.value()),f64::from(p.mod_resonance_step_85.value()),f64::from(p.mod_resonance_step_86.value()),f64::from(p.mod_resonance_step_87.value()),f64::from(p.mod_resonance_step_88.value()),f64::from(p.mod_resonance_step_89.value()),f64::from(p.mod_resonance_step_90.value()),f64::from(p.mod_resonance_step_91.value()),f64::from(p.mod_resonance_step_92.value()),f64::from(p.mod_resonance_step_93.value()),f64::from(p.mod_resonance_step_94.value()),f64::from(p.mod_resonance_step_95.value()),f64::from(p.mod_resonance_step_96.value()),f64::from(p.mod_resonance_step_97.value()),f64::from(p.mod_resonance_step_98.value()),f64::from(p.mod_resonance_step_99.value()),f64::from(p.mod_resonance_step_100.value()),f64::from(p.mod_resonance_step_101.value()),f64::from(p.mod_resonance_step_102.value()),f64::from(p.mod_resonance_step_103.value()),f64::from(p.mod_resonance_step_104.value()),f64::from(p.mod_resonance_step_105.value()),f64::from(p.mod_resonance_step_106.value()),f64::from(p.mod_resonance_step_107.value()),f64::from(p.mod_resonance_step_108.value()),f64::from(p.mod_resonance_step_109.value()),f64::from(p.mod_resonance_step_110.value()),f64::from(p.mod_resonance_step_111.value()),f64::from(p.mod_resonance_step_112.value()),f64::from(p.mod_resonance_step_113.value()),f64::from(p.mod_resonance_step_114.value()),f64::from(p.mod_resonance_step_115.value()),f64::from(p.mod_resonance_step_116.value()),f64::from(p.mod_resonance_step_117.value()),f64::from(p.mod_resonance_step_118.value()),f64::from(p.mod_resonance_step_119.value()),f64::from(p.mod_resonance_step_120.value()),f64::from(p.mod_resonance_step_121.value()),f64::from(p.mod_resonance_step_122.value()),f64::from(p.mod_resonance_step_123.value()),f64::from(p.mod_resonance_step_124.value()),f64::from(p.mod_resonance_step_125.value()),f64::from(p.mod_resonance_step_126.value()),f64::from(p.mod_resonance_step_127.value())]},
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,4)),speed:p.mod_pan_mseg_speed.value() as usize,fix:p.mod_pan_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_pan_lfo_status.value() as usize,
lfo_wave:p.mod_pan_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_pan_lfo_freq.value()),
lfo_sync:p.mod_pan_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_pan_lfo_phase.value()),
lfo_delay:f64::from(p.mod_pan_lfo_delay.value()),
lfo_fade:f64::from(p.mod_pan_lfo_fade.value()),
lfo_depth:f64::from(p.mod_pan_lfo_depth.value()),
step_status:p.mod_pan_step_status.value() as usize,
step_count:p.mod_pan_step_count.value() as usize,
step_freq:f64::from(p.mod_pan_step_freq.value()),
step_sync:p.mod_pan_step_sync.value() as usize,
step_smooth:f64::from(p.mod_pan_step_smooth.value()),
step_depth:f64::from(p.mod_pan_step_depth.value()),
step_retrigger:p.mod_pan_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_pan_step_0.value()),f64::from(p.mod_pan_step_1.value()),f64::from(p.mod_pan_step_2.value()),f64::from(p.mod_pan_step_3.value()),f64::from(p.mod_pan_step_4.value()),f64::from(p.mod_pan_step_5.value()),f64::from(p.mod_pan_step_6.value()),f64::from(p.mod_pan_step_7.value()),f64::from(p.mod_pan_step_8.value()),f64::from(p.mod_pan_step_9.value()),f64::from(p.mod_pan_step_10.value()),f64::from(p.mod_pan_step_11.value()),f64::from(p.mod_pan_step_12.value()),f64::from(p.mod_pan_step_13.value()),f64::from(p.mod_pan_step_14.value()),f64::from(p.mod_pan_step_15.value()),f64::from(p.mod_pan_step_16.value()),f64::from(p.mod_pan_step_17.value()),f64::from(p.mod_pan_step_18.value()),f64::from(p.mod_pan_step_19.value()),f64::from(p.mod_pan_step_20.value()),f64::from(p.mod_pan_step_21.value()),f64::from(p.mod_pan_step_22.value()),f64::from(p.mod_pan_step_23.value()),f64::from(p.mod_pan_step_24.value()),f64::from(p.mod_pan_step_25.value()),f64::from(p.mod_pan_step_26.value()),f64::from(p.mod_pan_step_27.value()),f64::from(p.mod_pan_step_28.value()),f64::from(p.mod_pan_step_29.value()),f64::from(p.mod_pan_step_30.value()),f64::from(p.mod_pan_step_31.value()),f64::from(p.mod_pan_step_32.value()),f64::from(p.mod_pan_step_33.value()),f64::from(p.mod_pan_step_34.value()),f64::from(p.mod_pan_step_35.value()),f64::from(p.mod_pan_step_36.value()),f64::from(p.mod_pan_step_37.value()),f64::from(p.mod_pan_step_38.value()),f64::from(p.mod_pan_step_39.value()),f64::from(p.mod_pan_step_40.value()),f64::from(p.mod_pan_step_41.value()),f64::from(p.mod_pan_step_42.value()),f64::from(p.mod_pan_step_43.value()),f64::from(p.mod_pan_step_44.value()),f64::from(p.mod_pan_step_45.value()),f64::from(p.mod_pan_step_46.value()),f64::from(p.mod_pan_step_47.value()),f64::from(p.mod_pan_step_48.value()),f64::from(p.mod_pan_step_49.value()),f64::from(p.mod_pan_step_50.value()),f64::from(p.mod_pan_step_51.value()),f64::from(p.mod_pan_step_52.value()),f64::from(p.mod_pan_step_53.value()),f64::from(p.mod_pan_step_54.value()),f64::from(p.mod_pan_step_55.value()),f64::from(p.mod_pan_step_56.value()),f64::from(p.mod_pan_step_57.value()),f64::from(p.mod_pan_step_58.value()),f64::from(p.mod_pan_step_59.value()),f64::from(p.mod_pan_step_60.value()),f64::from(p.mod_pan_step_61.value()),f64::from(p.mod_pan_step_62.value()),f64::from(p.mod_pan_step_63.value()),f64::from(p.mod_pan_step_64.value()),f64::from(p.mod_pan_step_65.value()),f64::from(p.mod_pan_step_66.value()),f64::from(p.mod_pan_step_67.value()),f64::from(p.mod_pan_step_68.value()),f64::from(p.mod_pan_step_69.value()),f64::from(p.mod_pan_step_70.value()),f64::from(p.mod_pan_step_71.value()),f64::from(p.mod_pan_step_72.value()),f64::from(p.mod_pan_step_73.value()),f64::from(p.mod_pan_step_74.value()),f64::from(p.mod_pan_step_75.value()),f64::from(p.mod_pan_step_76.value()),f64::from(p.mod_pan_step_77.value()),f64::from(p.mod_pan_step_78.value()),f64::from(p.mod_pan_step_79.value()),f64::from(p.mod_pan_step_80.value()),f64::from(p.mod_pan_step_81.value()),f64::from(p.mod_pan_step_82.value()),f64::from(p.mod_pan_step_83.value()),f64::from(p.mod_pan_step_84.value()),f64::from(p.mod_pan_step_85.value()),f64::from(p.mod_pan_step_86.value()),f64::from(p.mod_pan_step_87.value()),f64::from(p.mod_pan_step_88.value()),f64::from(p.mod_pan_step_89.value()),f64::from(p.mod_pan_step_90.value()),f64::from(p.mod_pan_step_91.value()),f64::from(p.mod_pan_step_92.value()),f64::from(p.mod_pan_step_93.value()),f64::from(p.mod_pan_step_94.value()),f64::from(p.mod_pan_step_95.value()),f64::from(p.mod_pan_step_96.value()),f64::from(p.mod_pan_step_97.value()),f64::from(p.mod_pan_step_98.value()),f64::from(p.mod_pan_step_99.value()),f64::from(p.mod_pan_step_100.value()),f64::from(p.mod_pan_step_101.value()),f64::from(p.mod_pan_step_102.value()),f64::from(p.mod_pan_step_103.value()),f64::from(p.mod_pan_step_104.value()),f64::from(p.mod_pan_step_105.value()),f64::from(p.mod_pan_step_106.value()),f64::from(p.mod_pan_step_107.value()),f64::from(p.mod_pan_step_108.value()),f64::from(p.mod_pan_step_109.value()),f64::from(p.mod_pan_step_110.value()),f64::from(p.mod_pan_step_111.value()),f64::from(p.mod_pan_step_112.value()),f64::from(p.mod_pan_step_113.value()),f64::from(p.mod_pan_step_114.value()),f64::from(p.mod_pan_step_115.value()),f64::from(p.mod_pan_step_116.value()),f64::from(p.mod_pan_step_117.value()),f64::from(p.mod_pan_step_118.value()),f64::from(p.mod_pan_step_119.value()),f64::from(p.mod_pan_step_120.value()),f64::from(p.mod_pan_step_121.value()),f64::from(p.mod_pan_step_122.value()),f64::from(p.mod_pan_step_123.value()),f64::from(p.mod_pan_step_124.value()),f64::from(p.mod_pan_step_125.value()),f64::from(p.mod_pan_step_126.value()),f64::from(p.mod_pan_step_127.value())]},
Destination {eg:mseg::Config::from_view(&crate::mod_view::View::bank(p,5)),speed:p.mod_amp_mseg_speed.value() as usize,fix:p.mod_amp_mseg_fix_tempo.value()>0.5,
lfo_status:p.mod_amp_lfo_status.value() as usize,
lfo_wave:p.mod_amp_lfo_wave.value() as usize,
lfo_freq:f64::from(p.mod_amp_lfo_freq.value()),
lfo_sync:p.mod_amp_lfo_sync.value() as usize,
lfo_phase:f64::from(p.mod_amp_lfo_phase.value()),
lfo_delay:f64::from(p.mod_amp_lfo_delay.value()),
lfo_fade:f64::from(p.mod_amp_lfo_fade.value()),
lfo_depth:f64::from(p.mod_amp_lfo_depth.value()),
step_status:p.mod_amp_step_status.value() as usize,
step_count:p.mod_amp_step_count.value() as usize,
step_freq:f64::from(p.mod_amp_step_freq.value()),
step_sync:p.mod_amp_step_sync.value() as usize,
step_smooth:f64::from(p.mod_amp_step_smooth.value()),
step_depth:f64::from(p.mod_amp_step_depth.value()),
step_retrigger:p.mod_amp_step_retrigger.value()>0.5,
steps:[f64::from(p.mod_amp_step_0.value()),f64::from(p.mod_amp_step_1.value()),f64::from(p.mod_amp_step_2.value()),f64::from(p.mod_amp_step_3.value()),f64::from(p.mod_amp_step_4.value()),f64::from(p.mod_amp_step_5.value()),f64::from(p.mod_amp_step_6.value()),f64::from(p.mod_amp_step_7.value()),f64::from(p.mod_amp_step_8.value()),f64::from(p.mod_amp_step_9.value()),f64::from(p.mod_amp_step_10.value()),f64::from(p.mod_amp_step_11.value()),f64::from(p.mod_amp_step_12.value()),f64::from(p.mod_amp_step_13.value()),f64::from(p.mod_amp_step_14.value()),f64::from(p.mod_amp_step_15.value()),f64::from(p.mod_amp_step_16.value()),f64::from(p.mod_amp_step_17.value()),f64::from(p.mod_amp_step_18.value()),f64::from(p.mod_amp_step_19.value()),f64::from(p.mod_amp_step_20.value()),f64::from(p.mod_amp_step_21.value()),f64::from(p.mod_amp_step_22.value()),f64::from(p.mod_amp_step_23.value()),f64::from(p.mod_amp_step_24.value()),f64::from(p.mod_amp_step_25.value()),f64::from(p.mod_amp_step_26.value()),f64::from(p.mod_amp_step_27.value()),f64::from(p.mod_amp_step_28.value()),f64::from(p.mod_amp_step_29.value()),f64::from(p.mod_amp_step_30.value()),f64::from(p.mod_amp_step_31.value()),f64::from(p.mod_amp_step_32.value()),f64::from(p.mod_amp_step_33.value()),f64::from(p.mod_amp_step_34.value()),f64::from(p.mod_amp_step_35.value()),f64::from(p.mod_amp_step_36.value()),f64::from(p.mod_amp_step_37.value()),f64::from(p.mod_amp_step_38.value()),f64::from(p.mod_amp_step_39.value()),f64::from(p.mod_amp_step_40.value()),f64::from(p.mod_amp_step_41.value()),f64::from(p.mod_amp_step_42.value()),f64::from(p.mod_amp_step_43.value()),f64::from(p.mod_amp_step_44.value()),f64::from(p.mod_amp_step_45.value()),f64::from(p.mod_amp_step_46.value()),f64::from(p.mod_amp_step_47.value()),f64::from(p.mod_amp_step_48.value()),f64::from(p.mod_amp_step_49.value()),f64::from(p.mod_amp_step_50.value()),f64::from(p.mod_amp_step_51.value()),f64::from(p.mod_amp_step_52.value()),f64::from(p.mod_amp_step_53.value()),f64::from(p.mod_amp_step_54.value()),f64::from(p.mod_amp_step_55.value()),f64::from(p.mod_amp_step_56.value()),f64::from(p.mod_amp_step_57.value()),f64::from(p.mod_amp_step_58.value()),f64::from(p.mod_amp_step_59.value()),f64::from(p.mod_amp_step_60.value()),f64::from(p.mod_amp_step_61.value()),f64::from(p.mod_amp_step_62.value()),f64::from(p.mod_amp_step_63.value()),f64::from(p.mod_amp_step_64.value()),f64::from(p.mod_amp_step_65.value()),f64::from(p.mod_amp_step_66.value()),f64::from(p.mod_amp_step_67.value()),f64::from(p.mod_amp_step_68.value()),f64::from(p.mod_amp_step_69.value()),f64::from(p.mod_amp_step_70.value()),f64::from(p.mod_amp_step_71.value()),f64::from(p.mod_amp_step_72.value()),f64::from(p.mod_amp_step_73.value()),f64::from(p.mod_amp_step_74.value()),f64::from(p.mod_amp_step_75.value()),f64::from(p.mod_amp_step_76.value()),f64::from(p.mod_amp_step_77.value()),f64::from(p.mod_amp_step_78.value()),f64::from(p.mod_amp_step_79.value()),f64::from(p.mod_amp_step_80.value()),f64::from(p.mod_amp_step_81.value()),f64::from(p.mod_amp_step_82.value()),f64::from(p.mod_amp_step_83.value()),f64::from(p.mod_amp_step_84.value()),f64::from(p.mod_amp_step_85.value()),f64::from(p.mod_amp_step_86.value()),f64::from(p.mod_amp_step_87.value()),f64::from(p.mod_amp_step_88.value()),f64::from(p.mod_amp_step_89.value()),f64::from(p.mod_amp_step_90.value()),f64::from(p.mod_amp_step_91.value()),f64::from(p.mod_amp_step_92.value()),f64::from(p.mod_amp_step_93.value()),f64::from(p.mod_amp_step_94.value()),f64::from(p.mod_amp_step_95.value()),f64::from(p.mod_amp_step_96.value()),f64::from(p.mod_amp_step_97.value()),f64::from(p.mod_amp_step_98.value()),f64::from(p.mod_amp_step_99.value()),f64::from(p.mod_amp_step_100.value()),f64::from(p.mod_amp_step_101.value()),f64::from(p.mod_amp_step_102.value()),f64::from(p.mod_amp_step_103.value()),f64::from(p.mod_amp_step_104.value()),f64::from(p.mod_amp_step_105.value()),f64::from(p.mod_amp_step_106.value()),f64::from(p.mod_amp_step_107.value()),f64::from(p.mod_amp_step_108.value()),f64::from(p.mod_amp_step_109.value()),f64::from(p.mod_amp_step_110.value()),f64::from(p.mod_amp_step_111.value()),f64::from(p.mod_amp_step_112.value()),f64::from(p.mod_amp_step_113.value()),f64::from(p.mod_amp_step_114.value()),f64::from(p.mod_amp_step_115.value()),f64::from(p.mod_amp_step_116.value()),f64::from(p.mod_amp_step_117.value()),f64::from(p.mod_amp_step_118.value()),f64::from(p.mod_amp_step_119.value()),f64::from(p.mod_amp_step_120.value()),f64::from(p.mod_amp_step_121.value()),f64::from(p.mod_amp_step_122.value()),f64::from(p.mod_amp_step_123.value()),f64::from(p.mod_amp_step_124.value()),f64::from(p.mod_amp_step_125.value()),f64::from(p.mod_amp_step_126.value()),f64::from(p.mod_amp_step_127.value())]},
]}}}
