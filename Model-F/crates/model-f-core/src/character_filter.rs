// SPDX-License-Identifier: GPL-3.0-or-later
// Geliştirici: Fazıl Irak | Düzenleme: 26.10.2026
//! Model F'nin özgün karakter filtreleri. Üretici kodu veya devre eşdeğerliği iddiası yoktur.
//! Mevcut Ladder ayrı yoldadır; bu modül yalnız seçilen alternatif filtreyi çalıştırır.

#[derive(Debug, Clone, Copy, Default)]
struct Svf { z1: f64, z2: f64 }
impl Svf {
    // TPT state-variable integratör: aynı durumdan LP/BP/HP/Notch çıkışları.
    fn tick(&mut self, x: f64, g: f64, damping: f64) -> [f64;4] {
        let a = 1.0 / (1.0 + g * (g + damping));
        let bp = a * (self.z1 + g * (x - self.z2));
        let lp = self.z2 + g * bp;
        self.z1 = (2.0 * bp - self.z1).clamp(-32.0,32.0);
        self.z2 = (2.0 * lp - self.z2).clamp(-32.0,32.0);
        let hp = x - damping * bp - lp;
        [lp, bp, hp, hp + lp]
    }
}
// Hızlı, tek değerli, sınırlı nonlinear transfer; integratörler sınırsız büyümez.
fn colour(x:f64)->f64 {
    let x=x.clamp(-3.0,3.0);let x2=x*x;
    x*(27.0+x2)/(27.0+9.0*x2)
}

#[derive(Debug,Clone)]
pub struct CharacterFilter {
    rate:f64,
    ladder:[f64;4],
    svf:[Svf;2],
    previous_input:f64,
    edge_low:f64,
    edge_hp:f64,
    acid_resonance:f64,
    acid_resonance_initialized:bool,
    shape:f64,
    shape_step:f64,
    dc_x:f64,
    dc_y:f64,
    dc_r:f64,
    coefficient_key:(u64,u64),
    g:f64,
    gh:f64,
}
impl CharacterFilter {
    pub fn new(rate:f64)->Self {
        Self {rate,ladder:[0.0;4],svf:[Svf::default();2],previous_input:0.0,edge_low:0.0,edge_hp:1.0-libm::exp(-std::f64::consts::TAU*80.0/(rate*2.0)),
            acid_resonance:0.0,acid_resonance_initialized:false,
            shape:0.0,shape_step:1.0-libm::exp(-1.0/(rate*0.02)),dc_x:0.0,dc_y:0.0,
            dc_r:libm::exp(-std::f64::consts::TAU*8.0/rate),coefficient_key:(0,0),g:0.0,gh:0.0}
    }
    pub fn reset(&mut self) {
        self.ladder=[0.0;4];self.svf=[Svf::default();2];self.previous_input=0.0;self.edge_low=0.0;
        self.acid_resonance_initialized=false;
        self.dc_x=0.0;self.dc_y=0.0;self.coefficient_key=(0,0);
    }
    // Mixer Norton akımı gerilime çevrilir. i0 bütün key tracking, zarf ve MSEG'yi içerir.
    // i0/(4*pi*Vt*C) küçük-sinyal ölçeği yalnız ortak Cutoff hareketini eşlemek içindir.
    pub fn tick(&mut self, mode:u8, current:f64, conductance:f64, i0:f64, resonance:f64, shape:f64)->f64 {
        let target=shape.clamp(0.0,1.0);
        self.shape += (target-self.shape)*self.shape_step;
        if (target-self.shape).abs()<1e-10 {self.shape=target;}
        let hz=(i0/(4.0*std::f64::consts::PI*0.02585*68e-9)).clamp(18.0,self.rate*0.20);
        let key=(hz.to_bits(),self.shape.to_bits());
        if key!=self.coefficient_key {
            self.coefficient_key=key;
            self.g=libm::tan(std::f64::consts::PI*hz/(self.rate*2.0));
            if mode==2 {self.gh=libm::tan(std::f64::consts::PI*(20.0*libm::pow(125.0,self.shape)).min(self.rate*0.15)/(self.rate*2.0));}
        }
        let x=(current/conductance.max(1e-9)).clamp(-12.0,12.0)*0.42;
        let mut r=resonance.clamp(0.0,1.0);
        // Fazıl Irak, 26.10.2026: yalnız Acid rezonansı 20 ms ile yumuşar.
        // İlk örnek mevcut ayara kurulur: yeni notanın başlangıcı gereksiz rampalanmaz.
        if mode==4 {
            if !self.acid_resonance_initialized {self.acid_resonance=r;self.acid_resonance_initialized=true;}
            else {self.acid_resonance+=(r-self.acid_resonance)*self.shape_step;}
            r=self.acid_resonance;
        }
        let mut sum=0.0;
        // İki alt örnek: doğrusal giriş enterpolasyonu, nonlinear aşamalar için 2x hız.
        for input in [(self.previous_input+x)*0.5,x] {
            let y=match mode {
                1=>{
                    // Edge: Odyssey Rev-I benzeri 12 dB hedefi; doğrulanmış donanım modeli değildir.
                    // Asimetrik giriş + iki kutuplu rezonanslı filtre, daha keskin harmonik doku.
                    // Ayrı bir HP kutbu düşük-orta bölgeyi sıkılaştırır (Odyssey yaklaşımı).
                    self.edge_low+=self.edge_hp*(input-self.edge_low);
                    let input=(colour((input-self.edge_low)*1.35+0.12)-colour(0.12))/1.12;
                    let out=self.svf[0].tick(input,self.g,1.6-1.45*r);
                    colour(out[0]*1.35)*(1.0-0.25*r)
                },
                2=>{
                    // Bite: ayrı rezonanslı HP ve LP; SHAPE high-pass frekansını 20–2500 Hz yapar.
                    let high=self.svf[0].tick(colour(input*1.7),self.gh,1.6-1.35*r)[2];
                    let low=self.svf[1].tick(colour(high*1.25),self.g,1.5-1.35*r)[0];
                    colour(low*1.8)*(1.0-0.40*r)
                },
                4=>{
                    // Acid: dört kutuplu nonlinear ladder; asimetrik giriş, güçlü rezonans ve drive.
                    // TB-303 esinli özgün ses hedefi; tam diyot devresi emülasyonu değildir.
                    let g=self.g/(1.0+self.g);let a=1.0-g;
                    // Fazıl Irak, 26.10.2026: her kata eklenen (1+0.09*r) kazancı
                    // döngüde dördüncü kuvvete çıkıp ~7.7 Emphasis'te erken salınım
                    // yaratıyordu. Birim küçük-sinyal eğimi, analitik TPT çözümündeki
                    // 1 + k*G^4 paydasıyla tutarlı; maksimum k=3.95, kritik 4'ün altında.
                    // Son çeyrekte eğim uçta sıfıra yaklaşır: az fare hareketi
                    // en yüksek rezonansa bir anda atlamaz. 0 ve 10 uçları korunur.
                    let top=((r-0.75)/0.25).clamp(0.0,1.0);
                    let expanded=if r>0.75 {0.75+0.25*(top+top*top-top*top*top)}else{r};
                    let feedback=3.95*expanded;
                    let memory=a*(self.ladder[3]+g*self.ladder[2]+g*g*self.ladder[1]+g*g*g*self.ladder[0]);
                    let mut y=((colour(input*2.4+0.16)-colour(0.16))-feedback*memory)/(1.0+feedback*g.powi(4));
                    for z in &mut self.ladder {
                        let v=(colour(y)-*z)*g;y=v+*z;*z=y+v;
                    }
                    y*(1.0+0.95*r)
                },
                5=>{
                    // VA Stack: dijital 12/24 dB low-pass; SHAPE eğimler arasında karışır.
                    let a=self.svf[0].tick(input,self.g,1.9-1.6*r)[0];
                    let b=self.svf[1].tick(a,self.g,1.4)[0];
                    (a*(1.0-self.shape)+b*self.shape)*1.3
                },
                _=>{
                    // State Variable: LP -> Notch -> HP -> BP arasında kesintisiz karışım.
                    let out=self.svf[0].tick(colour(input*0.8),self.g,1.8-1.55*r);
                    let outputs=[out[0],out[3],out[2],out[1]*(1.8-1.55*r)];
                    let at=self.shape*3.0;let ix=(at as usize).min(2);let t=at-ix as f64;
                    (outputs[ix]*(1.0-t)+outputs[ix+1]*t)*1.15
                }
            };
            sum+=y;
        }
        self.previous_input=x;
        let y=sum*0.5;
        // Nonlinear asimetri çıkış DC'sine dönüşmesin; VCA'dan önce temizle.
        let dc=y-self.dc_x+self.dc_r*self.dc_y;self.dc_x=y;self.dc_y=dc;
        // Aynı referans notada yaklaşık seviye eşleme; limiter/loudness normalizer değildir.
        let gain=match mode {1=>4.8,2=>2.4,3=>6.6,4=>4.6,_=>6.8};
        // Aşırı giriş/rezonansta VCA'ya sınırsız gerilim gönderilmez; yumuşak 5 V rayı.
        5.0*colour(dc*gain/5.0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn extremes_and_silence_are_finite_at_all_rates() {
        for sr in [8000.0,44100.0,48000.0,96000.0,192000.0] {for mode in 1..=5 {
            let mut f=CharacterFilter::new(sr);
            for n in 0..20000 {
                let i=if n<10000 {(n as f64*0.12).sin()*0.003}else{0.0};
                let y=f.tick(mode,i,0.00012,if n%1000<500{1e-9}else{0.002},1.0,(n as f64/20000.0).clamp(0.0,1.0));
                assert!(y.is_finite() && y.abs()<8.0,"mode {mode}, rate {sr}: {y}");
            }
        }}
    }
}

#[cfg(test)]
mod spectral_checks {
    use super::*;
    fn level(shape:f64,hz:f64)->f64 {
        let mut f=CharacterFilter::new(48000.0);let mut energy=0.0;
        for i in 0..24000 {
            let input=(std::f64::consts::TAU*hz*i as f64/48000.0).sin()*1e-5;
            let y=f.tick(3,input,0.00012,500.0*4.0*std::f64::consts::PI*0.02585*68e-9,0.3,shape);
            if i>=12000 {energy+=y*y;}
        }
        energy
    }
    #[test]
    fn state_variable_low_and_high_pass_exchange_frequency_regions() {
        assert!(level(0.0,100.0)>level(2.0/3.0,100.0)*20.0);
        assert!(level(2.0/3.0,4000.0)>level(0.0,4000.0)*20.0);
    }
}


/// Fazıl Irak, 26.10.2026: görsel yüksek-rezonans bölgesinin normalize başlangıcı.
/// Bir limiter veya mutlak güvenlik sınırı değildir. Ladder fabrikada yaklaşık
/// 7.5'te rejenerasyon hedefler; TPT türlerinde Q~2.5 ve üstü bölge işaretlenir.
/// Bite'de iki rezonans katı bulunduğundan erken olan LP eşiği kullanılır.
pub const fn emphasis_caution_start(mode:u8)->f64 {
    match mode {1=>0.83,2=>0.82,3=>0.90,4=>0.80,5=>0.94,_=>0.75}
}

#[cfg(test)]
mod resonance_regressions {
    use super::*;
    fn i0(hz:f64)->f64 {hz*4.0*std::f64::consts::PI*0.02585*68e-9}
    #[test]
    fn acid_small_signal_sweep_has_no_abrupt_early_oscillation() {
        // Fazıl Irak, 26.10.2026: 0.2.5'te 7.7/7.8 geçişi 500 Hz'de >25 dB,
        // 2 kHz'de >45 dB sıçrıyordu. 0.1 knob adımları şimdi 3 dB altında.
        for hz in [100.0,500.0,2000.0] {
            let mut previous=0.0_f64;
            for step in 0..=100 {
                let mut f=CharacterFilter::new(48000.0);let r=step as f64/100.0;let mut energy=0.0;
                for n in 0..24000 {
                    let x=1e-9*(std::f64::consts::TAU*hz*n as f64/48000.0).sin();
                    let y=f.tick(4,x,0.00012,i0(hz),r,0.0);
                    assert!(y.is_finite() && y.abs()<0.02);
                    if n>=12000 {energy+=y*y;}
                }
                let level=(energy/12000.0).sqrt();
                if previous>0.0 {let delta=20.0*(level/previous).log10();assert!(delta.abs()<3.0,"{hz} Hz, emphasis {r}: {delta} dB");}
                previous=level;
            }
        }
    }
    #[test]
    fn acid_live_resonance_is_smoothed_and_first_value_and_reset_are_exact() {
        for rate in [8000.0,44100.0,48000.0,96000.0,192000.0] {
            let mut f=CharacterFilter::new(rate);
            f.tick(4,0.0,0.00012,i0(500.0),0.2,0.0);assert_eq!(f.acid_resonance,0.2);
            f.tick(4,0.0,0.00012,i0(500.0),1.0,0.0);assert!(f.acid_resonance>0.2 && f.acid_resonance<0.21);
            for _ in 1..(rate*0.02) as usize {f.tick(4,0.0,0.00012,i0(500.0),1.0,0.0);}
            assert!((f.acid_resonance-(1.0-0.8*(-1.0_f64).exp())).abs()<0.001);
            f.reset();f.tick(4,0.0,0.00012,i0(500.0),0.7,0.0);assert_eq!(f.acid_resonance,0.7);
        }
    }
    #[test]
    fn acid_maximum_resonance_decays_after_excitation_at_all_rates() {
        for rate in [8000.0,44100.0,48000.0,96000.0,192000.0] {
            let mut f=CharacterFilter::new(rate);let mut on=0.0;let mut tail=0.0;
            for n in 0..(rate*1.2) as usize {
                let time=n as f64/rate;let x=if time<0.2 {1e-7*(std::f64::consts::TAU*1000.0*time).sin()}else{0.0};
                let y=f.tick(4,x,0.00012,i0(1000.0),1.0,0.0);assert!(y.is_finite()&&y.abs()<5.0);
                if (0.15..0.2).contains(&time) {on+=y*y;}
                if time>=1.15 {tail+=y*y;}
            }
            assert!(tail<on*0.01,"rate {rate}: tail {tail} / driven {on}");
        }
    }
}
