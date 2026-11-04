//! Original Model F output colour stage; independent drive and final trim.
//! First-order antiderivative antialiasing for the asymmetric tanh stage.
#[derive(Debug,Clone,Copy,PartialEq)]
pub struct Config {pub available:bool,pub enabled:bool,pub mic:bool,pub gain_db:f64,pub trim_db:f64}
impl Default for Config {fn default()->Self {Self {available:false,enabled:true,mic:false,gain_db:10.,trim_db:0.}}}
#[derive(Debug,Clone,Copy,Default)]
struct Channel {body:f64,previous:f64,dc_in:f64,dc_out:f64,antiderivative:f64,cache_valid:bool}
#[derive(Debug)]
pub struct Preamp {config:Config,channels:[Channel;2],gain:f64,trim:f64,blend:f64,mic:f64,target_gain:f64,target_trim:f64,smooth:f64,body:f64,dc:f64,last_threshold:f64,last_bias:f64}
impl Default for Preamp {fn default()->Self {Self {config:Default::default(),channels:[Channel::default();2],gain:1.,trim:1.,blend:0.,mic:0.,target_gain:1.,target_trim:1.,smooth:1.,body:0.,dc:0.,last_threshold:f64::NAN,last_bias:f64::NAN}}}
fn log_cosh(x:f64)->f64 {let a=x.abs();a+libm::log1p(libm::exp(-2.*a))-std::f64::consts::LN_2}
fn shape(x:f64,threshold:f64,bias:f64)->f64 {let b=libm::tanh(bias);threshold*(libm::tanh(x/threshold+bias)-b)/(1.-b*b)}
fn integral(x:f64,threshold:f64,bias:f64)->f64 {let b=libm::tanh(bias);(threshold*threshold*log_cosh(x/threshold+bias)-threshold*b*x)/(1.-b*b)}
impl Preamp {
    pub fn clear(&mut self) {self.channels=[Channel::default();2];}
    pub fn prepare(&mut self,rate:f64,c:Config) {
        self.config=c;self.channels=[Channel::default();2];
        self.smooth=1.-libm::exp(-1./(rate*0.020));self.body=1.-libm::exp(-std::f64::consts::TAU*120./rate);self.dc=libm::exp(-std::f64::consts::TAU*8./rate);
        self.configure(c);self.gain=self.target_gain;self.trim=self.target_trim;self.blend=if c.enabled {1.}else{0.};self.mic=if c.mic {1.}else{0.};
    }
    pub fn configure(&mut self,c:Config) {
        self.config=c;self.target_gain=10_f64.powf(c.gain_db.clamp(0.,36.)/20.);self.target_trim=10_f64.powf(c.trim_db.clamp(-60.,12.)/20.);
    }
    pub fn tick(&mut self,input:[f64;2])->[f64;2] {
        if !self.config.available {return input;}
        self.gain+=self.smooth*(self.target_gain-self.gain);self.trim+=self.smooth*(self.target_trim-self.trim);
        self.blend+=self.smooth*((if self.config.enabled {1.}else{0.})-self.blend);
        self.mic+=self.smooth*((if self.config.mic {1.}else{0.})-self.mic);
        let threshold=1.5-1.125*self.mic;let bias=0.02+0.06*self.mic;
        let transfer_changed=threshold.to_bits()!=self.last_threshold.to_bits()||bias.to_bits()!=self.last_bias.to_bits();self.last_threshold=threshold;self.last_bias=bias;
        let mut out=[0.;2];
        for (ch,x) in input.into_iter().enumerate() {
            let state=&mut self.channels[ch];let driven=x*self.gain;state.body+=self.body*(driven-state.body);
            let z=driven+state.body*(0.03+0.04*self.mic);let delta=z-state.previous;
            let wet=if delta.abs()<1e-5 {state.cache_valid=false;shape((z+state.previous)*0.5,threshold,bias)}else{
                let current=integral(z,threshold,bias);
                let previous=if state.cache_valid&&!transfer_changed {state.antiderivative}else{integral(state.previous,threshold,bias)};
                state.antiderivative=current;state.cache_valid=true;(current-previous)/delta
            };
            state.previous=z;let filtered=wet-state.dc_in+self.dc*state.dc_out;state.dc_in=wet;state.dc_out=filtered;
            out[ch]=(x+self.blend*(filtered-x))*self.trim;
            if !out[ch].is_finite() {out[ch]=0.;*state=Channel::default();}
        }out
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn bypass_keeps_trim_and_does_not_apply_gain() {
        let c=Config {available:true,enabled:false,gain_db:36.,trim_db:-6.,..Default::default()};let mut p=Preamp::default();p.prepare(48000.,c);
        for _ in 0..64 {let y=p.tick([0.25,-0.25]);assert!((y[0]-0.25*10_f64.powf(-6./20.)).abs()<1e-12);assert_eq!(y[0],-y[1]);}
    }
    #[test]
    fn mic_saturates_earlier_than_line_and_zero_input_stays_finite() {
        for rate in [44100.,48000.,96000.] {
            let mut mic=Preamp::default();let mut line=Preamp::default();let c=Config {available:true,enabled:true,gain_db:18.,..Default::default()};line.prepare(rate,c);mic.prepare(rate,Config {mic:true,..c});
            let(mut a,mut b)=(0_f64,0_f64);for i in 0..4096 {let x=0.15*libm::sin(i as f64*0.13);let l=line.tick([x,x]);let m=mic.tick([x,x]);a=a.max(l[0].abs());b=b.max(m[0].abs());assert!(l[0].is_finite()&&m[0].is_finite());}
            assert!(b<a*0.8,"MIC must saturate earlier: {b} / {a}");
            for _ in 0..4096 {assert!(mic.tick([0.;2]).iter().all(|x|x.is_finite()));}
        }
    }
    #[test]
    fn trim_scales_coloured_signal_without_changing_the_drive() {
        let c=Config {available:true,gain_db:24.,..Default::default()};let(mut a,mut b)=(Preamp::default(),Preamp::default());a.prepare(48000.,c);b.prepare(48000.,Config {trim_db:-12.,..c});
        for i in 0..512 {let x=0.2*libm::sin(i as f64*0.2);let a=a.tick([x,x]);let b=b.tick([x,x]);assert!((b[0]-a[0]*10_f64.powf(-12./20.)).abs()<1e-10);}
    }
}
