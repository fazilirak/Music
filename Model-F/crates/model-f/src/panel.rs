// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 26.10.2026
// Dosyanın görevi: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
//! Shared vector panel and hit regions: the plug-in and preview use this same drawing.
use crate::{params::ModelFParams, presets::Preset};
#[cfg(test)]
use nih_plug::prelude::*;
use std::fmt::Write;
pub const W: u32 = 1120;
pub const H: u32 = 760;
pub const MODES: [&str; 7] = [
    "POLY",
    "SUPERSAW",
    "OCTAVE",
    "FIFTH",
    "RING",
    "RING POLY",
    "MONO",
];
pub const LOGO_RECT: (f64, f64, f64, f64) = (28.0, 25.0, 210.0, 53.0);
pub const PANIC_RECT: (f64, f64, f64, f64) = (1010., 687., 82., 43.);
pub const ABOUT_RECT: (f64, f64, f64, f64) = (300.0, 190.0, 520.0, 380.0);
pub const ABOUT_CLOSE: (f64, f64, f64, f64) = (475.0, 495.0, 170.0, 38.0);
// in_rect alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn in_rect((x, y, w, h): (f64, f64, f64, f64), px: f64, py: f64) -> bool {
    px >= x && px <= x + w && py >= y && py <= y + h
}
pub const CHARACTER_NAMES:[&str;6]=["Ladder","Edge","Bite","State Variable","Acid","VA Stack"];
pub const CHARACTER_RECTS:[(f64,f64,f64,f64);6]=[(340.,85.,62.,22.),(406.,85.,52.,22.),(462.,85.,52.,22.),(518.,85.,106.,22.),(628.,85.,52.,22.),(684.,85.,82.,22.)];
// Fazıl Irak, 26.10.2026: sağdaki 22 px turuncu bölüm tek çift-yönlü seçim alanıdır.
pub const GLOBAL_TUNING_CYCLE_RECT:(f64,f64,f64,f64)=(193.0,697.0,22.0,28.0);
pub const GLOBAL_TUNING_CHOICES:[f32;5]=[432.0,440.0,442.0,444.0,528.0];
pub fn next_global_tuning(current:f32,reverse:bool)->f32 {
    // Ara değerlerde tıklanan yöndeki ilk hazır değere geç; uçta başa/sona sar.
    if reverse {GLOBAL_TUNING_CHOICES.iter().rev().copied().find(|v|*v<current-0.05).unwrap_or(528.0)}
    else {GLOBAL_TUNING_CHOICES.iter().copied().find(|v|*v>current+0.05).unwrap_or(432.0)}
}
pub fn global_tuning_cycle_at(p:&ModelFParams,v:&View,x:f64,y:f64,reverse:bool)->Option<f32> {
    (in_rect(GLOBAL_TUNING_CYCLE_RECT,x,y)&&control_enabled_for_view(p,v,"global_tuning"))
        .then(||next_global_tuning(p.global_tuning.value(),reverse))
}
pub fn global_tuning_colour(hz:f32)->&'static str {
    // Saklanan adım 0.1 Hz: yalnız ekranda tam hazır değeri gösteren konum renklenir.
    if (hz-432.0).abs()<0.05 {"#71c88b"}
    else if (hz-440.0).abs()<0.05 {"#ef5350"}
    else if (hz-442.0).abs()<0.05 || (hz-444.0).abs()<0.05 {"#ed6c46"}
    else if (hz-528.0).abs()<0.05 {"#69aefb"}
    else {"#c8cbc6"}
}
pub const WAVES: [&str; 6] = ["TRIANGLE", "SHARK", "SAW", "SQUARE", "PULSE 25", "PULSE 10"];
#[derive(Clone, Copy, Debug)]
pub struct Control {
    pub id: &'static str,
    pub label: &'static str,
    pub x: f64,
    pub y: f64,
    pub w: f64,
    pub h: f64,
    pub knob: bool,
}
impl Control {
    // Dikdörtgen değer alanları da knob ile aynı sürükleme sözleşmesini kullanır.
    pub fn draggable(&self)->bool {self.knob || matches!(self.id,"global_tuning"|"character_shape")}
    pub fn drag_normalized(&self,base:f32,up:f64,shift:bool)->f32 {
        let scale=if shift {0.0005} else {0.005};
        let scale=if self.id=="global_tuning" {(if shift {0.01}else{0.1}) / f64::from(crate::params::GLOBAL_TUNING_SPAN)} else {scale};
        (base+(up*scale) as f32).clamp(0.0,1.0)
    }

    // Fazıl Irak, 26.10.2026: fiziksel boyutu büyütmeden Acid Emphasis hareketini 2.5 kat hassaslaştır.
    pub fn drag_for_character(&self,base:f32,up:f64,shift:bool,character:u8)->f32 {
        self.drag_normalized(base,up*if self.id=="emphasis" && character==4 {0.4}else{1.0},shift)
    }
    pub fn wheel_step(&self,shift:bool,character:u8)->f32 {
        if self.id=="global_tuning" {0.1/crate::params::GLOBAL_TUNING_SPAN}
        else if self.id=="emphasis" && character==4 {if shift {0.001}else{0.005}}
        else if shift {0.002}else{0.02}
    }

    // hit: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    pub fn hit(&self, x: f64, y: f64) -> bool {
        if self.knob {
            (x - self.x).hypot(y - self.y) < self.w + (self.w * 0.4).clamp(6.0, 16.0)
        } else {
            x >= self.x && x <= self.x + self.w && y >= self.y && y <= self.y + self.h
        }
    }
}
macro_rules! k {
    ($id:literal,$label:literal,$x:expr,$y:expr,$r:expr) => {
        Control {
            id: $id,
            label: $label,
            x: $x as f64,
            y: $y as f64,
            w: $r as f64,
            h: 0.0,
            knob: true,
        }
    };
}
macro_rules! b {
    ($id:literal,$label:literal,$x:expr,$y:expr,$w:expr,$h:expr) => {
        Control {
            id: $id,
            label: $label,
            x: $x as f64,
            y: $y as f64,
            w: $w as f64,
            h: $h as f64,
            knob: false,
        }
    };
}
pub const CONTROLS: &[Control] = &[
    b!("global_tuning", "GLOBAL TUNING", 30, 697, 185, 28),
    b!("character_shape", "FILTER SHAPE", 470, 146, 72, 28),
    k!("volume", "OUTPUT", 1045, 59, 22),
    b!("mode", "VOICE", 48, 170, 286, 28),
    b!("waveform", "WAVE", 48, 241, 106, 28),
    b!("octave", "RANGE", 48, 294, 106, 28),
    b!("osc2_waveform", "WAVE", 232, 241, 106, 28),
    b!("osc2_octave", "RANGE", 232, 294, 106, 28),
    k!("osc1_level", "LEVEL", 72, 374, 18),
    k!("osc1_tune", "TUNE", 130, 374, 18),
    k!("osc2_level", "LEVEL", 256, 374, 18),
    k!("detune", "DETUNE", 314, 374, 18),
    b!("osc_sync", "SYNC", 171, 225, 48, 25),
    b!("fm_on", "FM", 171, 269, 48, 25),
    k!("fm_amount", "FM DEPTH", 195, 345, 13),
    k!("cutoff", "CUTOFF", 453, 245, 35),
    k!("emphasis", "EMPHASIS", 607, 245, 27),
    k!("contour", "CONTOUR", 616, 371, 21),
    k!("overload", "OVERLOAD", 430, 371, 21),
    b!("filter_mod", "FILTER MOD", 548, 146, 108, 28),
    k!("attack", "ATTACK", 758, 246, 21),
    k!("decay", "DECAY", 886, 246, 21),
    k!("sustain", "SUSTAIN", 1013, 246, 21),
    k!("audio_attack", "ATTACK", 758, 378, 21),
    k!("audio_decay", "DECAY", 886, 378, 21),
    k!("audio_sustain", "SUSTAIN", 1013, 378, 21),
    k!("noise", "NOISE", 523, 371, 21),
    b!("noise_color", "COLOR", 506, 294, 54, 23),
    b!("release", "DECAY", 922, 142, 150, 26),
    k!("mod_depth", "MOD DEPTH", 88, 567, 27),
    k!("lfo_rate", "LFO RATE", 195, 567, 27),
    k!("glide", "GLIDE", 302, 567, 24),
    b!("osc_mod", "OSC MOD", 48, 636, 126, 24),
    b!("arp_on", "ARP", 394, 532, 80, 30),
    b!("arp_mode", "DIRECTION", 494, 532, 132, 30),
    b!("arp_rate", "DIVISION", 646, 532, 100, 30),
    b!("arp_octaves", "OCTAVES", 766, 532, 100, 30),
    b!("arp_latch", "HOLD", 886, 532, 80, 30),
    b!("arp_sync", "HOST", 986, 532, 86, 30),
    k!("tempo", "TEMPO", 439, 616, 16),
    k!("arp_gate", "GATE", 568, 616, 16),
];
// escape alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn escape(s: &str) -> String {
    s.replace('&', "&amp;")
        .replace('<', "&lt;")
        .replace('>', "&gt;")
        .replace('"', "&quot;")
}
// text alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub(crate) fn text(
    s: &mut String,
    x: f64,
    y: f64,
    size: u32,
    color: &str,
    value: &str,
    anchor: &str,
) {
    let _ = write!(
        s,
        r###"<text x="{x}" y="{y}" fill="{color}" font-size="{size}" text-anchor="{anchor}">{}</text>"###,
        escape(value)
    );
}
// button alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn button(s: &mut String, x: f64, y: f64, w: f64, h: f64, label: &str, on: bool) {
    let (fill, stroke, color) = if on {
        ("#ed6c46", "#ed6c46", "#17191a")
    } else {
        ("#222628", "#3d4142", "#d8dad5")
    };
    let _ = write!(
        s,
        r###"<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="{fill}" stroke="{stroke}"/>"###
    );
    text(
        s,
        x + w / 2.0,
        y + h / 2.0 + 4.0,
        if w < 55.0 { 8 } else { 11 },
        color,
        label,
        "middle",
    );
}
// section alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn section(s: &mut String, x: f64, y: f64, w: f64, h: f64, n: &str, title: &str) {
    let _ = write!(
        s,
        r###"<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="8" fill="#1d2123" stroke="#353a3c"/>"###
    );
    text(s, x + 20.0, y + 28.0, 10, "#ed6c46", n, "start");
    text(s, x + 49.0, y + 28.0, 12, "#c8cbc6", title, "start");
}
// value alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: p.get → map_or → p.unmodulated_plain_value → v.round. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn value(p: &ModelFParams, id: &str) -> String {
    let v = p.effective(id);
    match id {
        "global_tuning"=>format!("{v:.1} Hz"),
        "character_shape"=>if p.character.value().round()==5.0 {if v<0.01 {"12 dB".into()} else if v>0.99 {"24 dB".into()} else {"12/24 MIX".into()}} else if p.character.value().round()==2.0 {format!("HP {:.0} Hz",20.0*libm::pow(125.0,f64::from(v)))} else {
            let at=v*3.0;let names=["LP","NOTCH","HP","BP"];let i=(at as usize).min(3);
            if (at-i as f32).abs()<0.03 || i==3 {names[i].into()} else {format!("{} / {}",names[i],names[i+1])}
        },
        "osc_sync" | "fm_on" => if v > 0.5 { "ON" } else { "OFF" }.into(),
        "osc1_tune" => format!("{v:+.1} st"),
        "osc1_level" | "osc2_level" => format!("{:.0}%", v * 10.0),
        "fm_amount" => format!("{:.0}%", v * 100.0),
        "osc2_waveform" => WAVES[(v.round() as usize).min(5)].into(),
        "osc2_octave" => ["32′", "16′", "8′", "4′", "2′", "1′"][(v.round() as usize).min(5)].into(),
        "mode" => if p.character.value().round()==5.0 {"SUPERSAW".into()} else {MODES[(v.round() as usize).min(6)].into()},
        "waveform" => {
            if p.mode.value().round() == 1.0 || p.character.value().round()==5.0 {
                "SAW × 7".into()
            } else {
                WAVES[(v.round() as usize).min(5)].into()
            }
        }
        "octave" => ["32′", "16′", "8′", "4′", "2′", "1′"][(v.round() as usize).min(5)].into(),
        "noise_color" => if v > 0.5 { "PINK" } else { "WHITE" }.into(),
        "arp_mode" => ["UP", "DOWN", "UP / DOWN", "RANDOM"][(v.round() as usize).min(3)].into(),
        "arp_rate" => crate::arp::DIVISION_NAMES[(v.round() as usize).min(5)].into(),
        "arp_octaves" => format!("{} OCT", v.round() as usize),
        "arp_gate" => format!("{:.0}%", v * 100.0),
        "detune" => format!("{v:.1}%"),
        "tempo" => format!("{v:.1}"),
        "release" | "filter_mod" | "osc_mod" | "arp_on" | "arp_latch" | "arp_sync" => {
            if v > 0.5 { "ON" } else { "OFF" }.into()
        }
        _ => format!("{v:.2}"),
    }
}
#[derive(Default, Debug)]
pub struct View {
    pub envelope: bool,
    pub env: crate::mseg_ui::State,
    pub about: bool,
    pub name: String,
    pub category: String,
    pub dirty: bool,
    pub status: String,
    pub meter: f32,
    pub overload: f32,
    pub browser: bool,
    pub delete_name: Option<String>,
    pub filter: String,
    pub search: String,
    pub rows: Vec<Preset>,
    pub offset: usize,
    pub save: bool,
    pub save_name: String,
    pub save_category: usize,
    pub hover: Option<usize>,
    pub keys: u32,
}
impl View {
    // Fazıl Irak — 25.10.2026: Görsel pasiflik ve giriş engeli aynı durumdan türetilir.
    pub fn header_enabled(&self) -> bool { !self.browser && !self.envelope && !self.save && !self.about }
    pub fn multi_button_enabled(&self) -> bool { !self.browser && !self.save && !self.about }
    pub fn blocks_click(&self,x:f64,y:f64) -> bool {
        (!self.header_enabled() && in_rect((330.,0.,762.,117.),x,y)) ||
        (!self.multi_button_enabled() && in_rect(crate::mseg_ui::OPEN,x,y))
    }

    // open_about alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
    pub fn open_about(&mut self) {
        self.about = true;
        self.hover = None;
    }
    // about_click alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
    pub fn about_click(&mut self, x: f64, y: f64) {
        if in_rect(ABOUT_CLOSE, x, y) || !in_rect(ABOUT_RECT, x, y) {
            self.about = false;
        }
    }
}
// svg alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: text → button → v.category.to_uppercase → s.push_str. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn svg(p: &ModelFParams, v: &View) -> String {
    let mut s = format!(
        r##"<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}"><rect width="1120" height="760" rx="12" fill="#141819"/><g font-family="URW Gothic, sans-serif">"##
    );
    text(&mut s, 30.0, 66.0, 43, "#e5e7df", "Model F", "start");
    text(
        &mut s,
        32.0,
        91.0,
        10,
        "#8d9799",
        "FAZIL IRAK  /  CIRCUIT SYNTHESIZER",
        "start",
    );
    if !v.header_enabled() {s.push_str(r##"<g id="preset-header-disabled" opacity="0.3">"##);}
    text(&mut s, 346.0, 29.0, 9, "#8d9799", "SOUND LIBRARY", "start");
    button(
        &mut s,
        340.0,
        40.0,
        430.0,
        38.0,
        &format!("{}{}", v.name, if v.dirty { " •" } else { "" }),
        false,
    );
    // Preset kategori satırının yerine küçük, doğrudan seçilen karakter etiketleri.
    for (i,rect) in CHARACTER_RECTS.iter().enumerate() {
        button(&mut s,rect.0,rect.1,rect.2,rect.3,CHARACTER_NAMES[i],p.character.value().round() as usize==i);
    }
    for (x, label) in [(782.0, "‹"), (820.0, "›")] {
        button(&mut s, x, 40.0, 30.0, 38.0, label, false);
    }
    for (x, label) in [(868.0, "SAVE"), (934.0, "LOAD")] {
        button(&mut s, x, 40.0, 56.0, 38.0, label, false);
    }
    text(&mut s, 871.0, 96.0, 9, "#8d9799", "EXPORT .MODELF", "start");
    if !v.header_enabled() {s.push_str("</g>");}
    s.push_str(r##"<path d="M28 118H1092" stroke="#ed6c46" stroke-width="2"/><path d="M28 123H1092" stroke="#dce0d8" stroke-width="2"/>"##);
    section(&mut s, 28.0, 132.0, 326.0, 310.0, "01", "OSCILLATORS");
    section(&mut s, 374.0, 132.0, 300.0, 310.0, "02", "FILTER");
    section(&mut s, 694.0, 132.0, 398.0, 310.0, "03", "ENVELOPES");
    section(&mut s, 28.0, 464.0, 326.0, 202.0, "04", "MODULATION");
    section(&mut s, 374.0, 464.0, 718.0, 210.0, "05", "ARPEGGIATOR");
    // Fazıl Irak, 24.10.2026: seçiciler, knob tick ve değerleri çerçevelerin içinde boşluk bırakır.
    // İki oscillator ve iki bağımsız contour, aynı 1120×760 panel içinde ayrılır.
    for (x, title) in [(40., "OSC 1"), (224., "OSC 2")] {
        let _ = write!(s, r##"<rect x="{x}" y="206" width="122" height="226" rx="5" fill="#191e20" stroke="#343d40"/>"##);
        text(&mut s, x + 61., 221., 10, "#e5e7df", title, "middle");
    }
    for (y, title) in [(176., "FILTER ENV"), (308., "AUDIO ENV")] {
        let _ = write!(s, r##"<rect x="710" y="{y}" width="366" height="126" rx="5" fill="#191e20" stroke="#3e474a"/>"##);
        text(&mut s, 723., y + 16., 10, "#e5e7df", title, "start");
    }
    for (i, c) in CONTROLS.iter().enumerate() {
        
        let n = f64::from(p.display_normalized(c.id));
        let enabled = control_enabled_for_view(p, v, c.id);
        if !enabled { s.push_str(r##"<g opacity="0.3">"##); }
        let active = v.hover == Some(i);
        let accent = if active { "#ff956f" } else { "#ed6c46" };
        if c.id=="global_tuning" {
            button(&mut s,c.x,c.y,c.w,c.h,"",false);
            // Sağ köşeler mevcut 4 px yuvarlamayı izler; sadece sağ şerit turuncudur.
            s.push_str(r##"<path data-role="global-tuning-cycle" d="M193 697H211A4 4 0 0 1 215 701V721A4 4 0 0 1 211 725H193Z" fill="#ed6c46"/>"##);
            text(&mut s,c.x+8.0,c.y+18.0,8,"#afb8b8","GLOBAL TUNING","start");
            text(&mut s,185.0,c.y+18.0,11,global_tuning_colour(p.global_tuning.value()),&value(p,c.id),"end");
            text(&mut s,204.0,c.y+18.0,12,"#151b1c","›","middle");
        } else if c.id=="character_shape" {
            button(&mut s,c.x,c.y,c.w,c.h,&value(p,c.id),false);
        } else if c.knob {
            let r = c.w;
            let caution=if c.id=="emphasis" {Some(model_f_core::character_filter::emphasis_caution_start(p.character.value().round() as u8))}else{None};
            for t in 0..=10 {
                let a = (-225.0 + 27.0 * f64::from(t)).to_radians();
                let (sn, cs) = a.sin_cos();
                let _ = write!(
                    s,
                    r###"<path d="M {} {} L {} {}" stroke="{}" stroke-width="1.4"/>"###,
                    c.x + cs * (r + 7.0),
                    c.y + sn * (r + 7.0),
                    c.x + cs * (r + 11.0),
                    c.y + sn * (r + 11.0),
                    if caution.is_some_and(|start|f64::from(t)/10.0>=start) {"#ed6c46"}else{"#525b5e"}
                );
            }
            let a = (-225.0 + n * 270.0).to_radians();
            let (sn, cs) = a.sin_cos();
            let _ = write!(
                s,
                r##"<circle cx="{}" cy="{}" r="{}" fill="#101415" stroke="#3e474a" stroke-width="2"/><circle cx="{}" cy="{}" r="{}" fill="#292f31"/><path d="M {} {} L {} {}" stroke="{}" stroke-width="3" stroke-linecap="round"/>"##,
                c.x,
                c.y,
                r,
                c.x,
                c.y,
                r - 5.0,
                c.x + cs * (r * 0.48),
                c.y + sn * (r * 0.48),
                c.x + cs * (r - 5.0),
                c.y + sn * (r - 5.0),
                accent
            );
            if let Some(start)=caution {
                // Fazıl Irak, 26.10.2026: turuncu parça gri dış halkanın üstüne aynı yarıçap/kalınlıkla çizilir.
                let radius=r;let a=(-225.0+270.0*start).to_radians();let b=45.0f64.to_radians();
                let _=write!(s,r##"<path data-role="emphasis-high-resonance" data-start="{start:.2}" d="M {} {} A {radius} {radius} 0 0 1 {} {}" fill="none" stroke="#ed6c46" stroke-width="2" stroke-linecap="butt"/>"##,
                    c.x+radius*a.cos(),c.y+radius*a.sin(),c.x+radius*b.cos(),c.y+radius*b.sin());
            }
            text(
                &mut s,
                c.x,
                c.y - r - 19.0,
                10,
                "#afb8b8",
                c.label,
                "middle",
            );
            text(
                &mut s,
                c.x,
                c.y + r + if c.id == "volume" { 28.0 } else { 30.0 },
                11,
                accent,
                &value(p, c.id),
                "middle",
            );
        } else {
            let switch = matches!(
                c.id,
                "release" | "filter_mod" | "osc_mod" | "arp_on" | "arp_latch" | "arp_sync" | "osc_sync" | "fm_on"
            );
            let label = if c.id == "mode" { format!("VOICE · {}", value(p,c.id)) } else if switch {
                format!("{}  {}", c.label, value(p, c.id))
            } else {
                value(p, c.id)
            };
            button(&mut s, c.x, c.y, c.w, c.h, &label, switch && n > 0.5);
            if !switch && c.id != "mode" {
                text(&mut s, c.x, c.y - 9.0, 9, "#899598", c.label, "start");
            }
        }
        if !enabled { s.push_str("</g>"); }
    }
    // The lamp follows the return saturation stage's input drive, not output volume.
    let lamp = v.overload.clamp(0.0, 1.0);
    let _ = write!(
        s,
        r##"<circle cx="472" cy="349" r="10" fill="#ed6c46" opacity="{}"/><circle cx="472" cy="349" r="6" fill="#43271e" stroke="#644235"/><circle cx="472" cy="349" r="5" fill="#ff794b" opacity="{}"/>"##,
        lamp * 0.22,
        lamp
    );
    text(
        &mut s,
        191.0,
        652.0,
        9,
        "#8c999b",
        "FREE-RUNNING LFO",
        "start",
    );
    for n in 0..16 {
        let x = 673 + n * 24;
        let on = n < ((v.meter.clamp(0.0, 1.0) * 16.0) as i32);
        let _ = write!(
            s,
            r###"<rect x="{x}" y="576" width="15" height="25" rx="2" fill="{}"/>"###,
            if on { "#ed6c46" } else { "#313b3d" }
        );
    }
    text(
        &mut s,
        674.0,
        625.0,
        9,
        "#899598",
        "SAMPLE-CLOCKED  /  MIDI + HOST TEMPO",
        "start",
    );


    text(
        &mut s,
        30.0,
        748.0,
        10,
        "#a8b0ae",
        &v.status.chars().take(100).collect::<String>(),
        "start",
    );
    // Two octaves, consistent visual and hit-test coordinates.
    for n in 0..24u32 {
        let x = 420 + n * 24;
        let black = [1, 3, 6, 8, 10].contains(&(n % 12));
        let fill = if v.keys & (1 << n) != 0 {
            "#ed6c46"
        } else if black {
            "#2b3437"
        } else {
            "#c3cbc7"
        };
        let _ = write!(
            s,
            r###"<rect x="{x}" y="687" width="21" height="43" rx="3" fill="{fill}"/>"###
        );
    }
    let (x, y, w, h) = PANIC_RECT;
    button(&mut s, x, y, w, h, "PANIC", false);
    let (x, y, w, h) = crate::mseg_ui::OPEN;
    if !v.multi_button_enabled() {s.push_str(r##"<g id="multi-button-disabled" opacity="0.3">"##);}
    button(
        &mut s,
        x,
        y,
        w,
        h,
        "MULTI-STAGE ENV.",
        p.mseg_enable.value() > 0.5,
    );
    if p.mseg_enable.value() <= 0.5 && p.mseg_amount.value().abs() > 0.0005 {
        // AD açık ve amount etkin: logoyla aynı kırık beyaz. Multi hâlâ turuncudur.
        let _ = write!(s, r##"<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="#e5e7df" stroke="#e5e7df"/>"##);
        text(&mut s, x+w/2., y+h/2.+4., 11, "#17191a", "MULTI-STAGE ENV.", "middle");
    }
    if !v.multi_button_enabled() {s.push_str("</g>");}
    if v.envelope {
        crate::mseg_ui::draw(&mut s, p, &v.env);
    }
    if v.browser {
        let _ = write!(
            s,
            r##"<rect x="28" y="132" width="1064" height="524" rx="8" fill="#1b2123" stroke="#596265"/>"##
        );
        text(&mut s, 49.0, 167.0, 16, "#e2e5de", "SOUND LIBRARY", "start");
        text(&mut s, 1067.0, 165.0, 11, "#ed6c46", "CLOSE ×", "end");
        for (i, c) in ["Bass", "Lead", "Key", "Pad", "FX", "Arp", "User"]
            .iter()
            .enumerate()
        {
            button(
                &mut s,
                48.0 + i as f64 * 100.0,
                185.0,
                88.0,
                30.0,
                &c.to_uppercase(),
                v.filter == *c,
            );
        }
        button(
            &mut s,
            772.0,
            185.0,
            300.0,
            30.0,
            &format!("SEARCH: {}_", v.search),
            false,
        );
        let count = v.rows.len();
        for (i, preset) in v.rows.iter().skip(v.offset).take(20).enumerate() {
            let x = 48.0 + (i / 10) as f64 * 520.0;
            let y = 234.0 + (i % 10) as f64 * 36.0;
            button(
                &mut s,
                x,
                y,
                if v.filter=="User" && preset.source_path.is_some() {464.0} else {504.0},
                30.0,
                &format!("{:02}   {}", v.offset + i + 1, preset.name),
                preset.name == v.name,
            );
            if v.filter=="User" && preset.source_path.is_some() {
                button(&mut s,x+468.,y,36.,30.,"×",false);
                text(&mut s,x+486.,y+21.,18,"#ed6c46","×","middle");
            }
        }
        text(
            &mut s,
            50.0,
            637.0,
            10,
            "#899598",
            &format!("{count} SOUNDS  /  CLICK TO LOAD  /  ESC TO CLOSE  /  SCROLL TO PAGE"),
            "start",
        );
    }
    if v.save {
        let _ = write!(
            s,
            r##"<rect width="1120" height="760" fill="#070b0c" opacity=".82"/><rect x="310" y="245" width="500" height="260" rx="9" fill="#22292b" stroke="#566265"/>"##
        );
        text(
            &mut s,
            340.0,
            285.0,
            18,
            "#e0e4db",
            "SAVE YOUR SOUND",
            "start",
        );
        text(
            &mut s,
            340.0,
            318.0,
            10,
            "#9aabae",
            "NAME  /  CLICK TO EDIT",
            "start",
        );
        button(
            &mut s,
            340.0,
            332.0,
            440.0,
            40.0,
            &format!("{}_", v.save_name),
            false,
        );
        button(
            &mut s,
            340.0,
            393.0,
            200.0,
            34.0,
            crate::presets::CATEGORIES[v.save_category],
            false,
        );
        button(&mut s, 562.0, 393.0, 100.0, 34.0, "CANCEL", false);
        button(&mut s, 680.0, 393.0, 100.0, 34.0, "SAVE", true);
        text(
            &mut s,
            340.0,
            466.0,
            10,
            "#ed6c46",
            &v.status.chars().take(60).collect::<String>(),
            "start",
        );
    }
    if v.about {
        let (x, y, w, h) = ABOUT_RECT;
        let _ = write!(
            s,
            r##"<rect width="1120" height="760" fill="#070b0c" opacity=".86"/><rect x="{x}" y="{y}" width="{w}" height="{h}" rx="12" fill="#22292b" stroke="#566265"/>"##
        );
        text(&mut s, 560.0, 237.0, 11, "#ed6c46", "ABOUT", "middle");
        text(&mut s, 560.0, 300.0, 42, "#e5e7df", "Model F", "middle");
        text(&mut s, 560.0, 337.0, 17, "#ed6c46", "FAZIL IRAK", "middle");
        text(
            &mut s,
            560.0,
            375.0,
            11,
            "#afb8b8",
            &format!("VERSION {}", env!("CARGO_PKG_VERSION")),
            "middle",
        );
        text(
            &mut s,
            560.0,
            416.0,
            12,
            "#d8dad5",
            "Analog circuit synthesizer",
            "middle",
        );

        let (bx, by, bw, bh) = ABOUT_CLOSE;
        button(&mut s, bx, by, bw, bh, "CLOSE", true);
    }
    if let Some(name)=&v.delete_name {
        s.push_str(r##"<g id="delete-user-preset-confirm"><rect width="1120" height="760" fill="#070b0c" opacity="0.82"/><rect x="310" y="260" width="500" height="240" rx="9" fill="#22292b" stroke="#566265"/>"##);
        text(&mut s,340.,300.,18,"#e5e7df","DELETE USER PRESET?","start");
        text(&mut s,340.,344.,15,"#ed6c46",&name.chars().take(45).collect::<String>(),"start");
        text(&mut s,340.,382.,11,"#a8b0ae","Moved to Presets/.deleted so you can restore the file.","start");
        button(&mut s,520.,430.,120.,34.,"CANCEL",false);
        button(&mut s,660.,430.,120.,34.,"DELETE",true);
        s.push_str("</g>");
    }
    s.push_str("</g></svg>");
    s
}

// Hazırlanmış durumdan ses veya panel çıktısını üretir; bu dosyanın DSP/GUI bağlamını aşağıdaki gövdeden ayırın.
// Gövdedeki başlıca çağrılar: resvg::usvg::fontdb::Database::new → db.load_font_data → to_vec → std::sync::Arc::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn render(svg: &str, width: u32, height: u32) -> Result<resvg::tiny_skia::Pixmap, String> {
    let mut db = resvg::usvg::fontdb::Database::new();
    db.load_font_data(
        include_bytes!("../../../third_party/urw-gothic/URWGothic-Book.otf").to_vec(),
    );
    let opt = resvg::usvg::Options {
        fontdb: std::sync::Arc::new(db),
        ..Default::default()
    };
    let tree = resvg::usvg::Tree::from_str(svg, &opt).map_err(|e| e.to_string())?;
    let mut pixels = resvg::tiny_skia::Pixmap::new(width, height).ok_or("Invalid panel size")?;
    resvg::render(
        &tree,
        resvg::tiny_skia::Transform::from_scale(width as f32 / W as f32, height as f32 / H as f32),
        &mut pixels.as_mut(),
    );
    Ok(pixels)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn overlay_batch_states_render_without_breaking_native_save_flow() {
        let p=ModelFParams::default();
        for scene in ["multi-ad","multi-on","user-library","delete-confirm"] {
            let multi=scene.starts_with("multi");
            let p=if scene=="multi-on" {ModelFParams {mseg_enable:FloatParam::new("Multi",1.,FloatRange::Linear{min:0.,max:1.}),..ModelFParams::default()}} else {ModelFParams::default()};
            let mut v=View {name:"Example User Bass".into(),category:"Bass".into(),filter:"User".into(),envelope:multi,browser:!multi,..View::default()};
            let mut user=Preset::capture(&p,"Example User Bass".into(),"Bass".into());user.source_path=Some("preview-only.modelf".into());v.rows=vec![user];
            if scene=="delete-confirm" {v.delete_name=Some("Example User Bass".into());}
            let image=svg(&p,&v);
            assert!(image.contains("preset-header-disabled"));
            assert_eq!(image.contains("multi-button-disabled"),!multi);
            assert_eq!(image.contains("delete-user-preset-confirm"),scene=="delete-confirm");
            let px=render(&image,W,H).unwrap();render(&image,W*2,H*2).unwrap();
            if let Some(dir)=std::env::var_os("MODEL_F_UI_PREVIEW_DIR") {
                let dir=std::path::PathBuf::from(dir);std::fs::create_dir_all(&dir).unwrap();
                px.save_png(dir.join(format!("{scene}.png"))).unwrap();
            }
        }
        assert!(!View::default().save);
        assert!(control_enabled_for_view(&p,&View::default(),"volume"));
    }
    #[test]
    fn overlay_controls_block_clicks_and_restore_after_close() {
        let p=ModelFParams::default();let mut v=View::default();
        assert!(v.header_enabled());assert!(v.multi_button_enabled());
        for point in [(500.,60.),(797.,60.),(900.,60.),(1050.,60.)] {assert!(!v.blocks_click(point.0,point.1));}
        v.envelope=true;
        for point in [(500.,60.),(797.,60.),(900.,60.),(1050.,60.)] {assert!(v.blocks_click(point.0,point.1));}
        assert!(!control_enabled_for_view(&p,&v,"volume"));assert!(v.multi_button_enabled());
        v.envelope=false;v.browser=true;assert!(!v.multi_button_enabled());assert!(v.blocks_click(320.,705.));
        assert!(svg(&p,&v).contains("multi-button-disabled"));
        v.browser=false;assert!(v.header_enabled());assert!(control_enabled_for_view(&p,&v,"volume"));
    }
    #[test]
    fn delete_buttons_only_hit_user_rows_not_factory_or_row_gaps() {
        let mut v=View {browser:true,filter:"User".into(),..View::default()};
        let mut user=Preset::capture(&ModelFParams::default(),"User".into(),"Bass".into());user.source_path=Some("user.modelf".into());
        v.rows=vec![user;12];
        assert_eq!(user_delete_hit(&v,530.,245.),Some(0));assert_eq!(user_delete_hit(&v,1050.,245.),Some(10));
        assert_eq!(user_delete_hit(&v,530.,266.),None);assert_eq!(user_delete_hit(&v,200.,245.),None);
        v.filter="Bass".into();assert_eq!(user_delete_hit(&v,530.,245.),None);
        v.filter="User".into();v.rows=crate::presets::factory();assert_eq!(user_delete_hit(&v,530.,245.),None);
    }
    #[test]
    // Regresyon senaryosu about_opens_and_closes_without_changing_synth_parameters: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: ModelFParams::default → p.controls → View::default → in_rect. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn about_opens_and_closes_without_changing_synth_parameters() {
        let p = ModelFParams::default();
        let before = p.controls();
        let mut view = View::default();
        assert!(in_rect(LOGO_RECT, 100.0, 50.0));
        view.open_about();
        assert!(view.about);
        view.about_click(560.0, 330.0);
        assert!(view.about);
        let drawing = svg(&p, &view);
        assert!(drawing.contains("FAZIL IRAK") && drawing.contains(concat!("VERSION ", env!("CARGO_PKG_VERSION"))));
        assert!(render(&drawing, W, H).is_ok());
        view.about_click(560.0, 514.0);
        assert!(!view.about);
        view.open_about();
        view.about_click(20.0, 20.0);
        assert!(!view.about);
        assert_eq!(before, p.controls());
    }
    #[test]
    // Regresyon senaryosu rendered_labels_keep_clearance_from_text_and_controls: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: texts → g.children → Node::Group → Node::Text. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn rendered_labels_keep_clearance_from_text_and_controls() {
        use resvg::usvg::{Group, Node};
        // texts alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
        fn texts(g: &Group, out: &mut Vec<(String, resvg::usvg::Rect)>) {
            for n in g.children() {
                match n {
                    Node::Group(g) => texts(g, out),
                    Node::Text(t) => out.push((
                        t.chunks().iter().map(|c| c.text()).collect::<String>(),
                        t.abs_bounding_box(),
                    )),
                    _ => (),
                }
            }
        }
        // overlaps alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
        fn overlaps(a: (f64, f64, f64, f64), b: (f64, f64, f64, f64), gap: f64) -> bool {
            a.0 < b.0 + b.2 + gap
                && a.0 + a.2 + gap > b.0
                && a.1 < b.1 + b.3 + gap
                && a.1 + a.3 + gap > b.1
        }
        for mode in [6.0, 1.0] {
            let p = ModelFParams {
                mode: FloatParam::new(
                    "Voice Mode",
                    mode,
                    FloatRange::Linear { min: 0.0, max: 6.0 },
                ),
                ..Default::default()
            };
            let mut db = resvg::usvg::fontdb::Database::new();
            db.load_font_data(
                include_bytes!("../../../third_party/urw-gothic/URWGothic-Book.otf").to_vec(),
            );
            let options = resvg::usvg::Options {
                fontdb: std::sync::Arc::new(db),
                ..Default::default()
            };
            let tree = resvg::usvg::Tree::from_str(
                &svg(
                    &p,
                    &View {
                        name: "Night Driver".into(),
                        category: "Bass".into(),
                        ..Default::default()
                    },
                ),
                &options,
            )
            .unwrap();
            let mut labels = Vec::new();
            texts(tree.root(), &mut labels);
            assert!(
                labels.len() > 50,
                "Measure the rendered text, including the output value"
            );
            assert!(labels.iter().any(|(name, _)| name == "EMPHASIS"));
            let bounds: Vec<_> = labels
                .iter()
                .map(|(_, r)| {
                    (
                        f64::from(r.x()),
                        f64::from(r.y()),
                        f64::from(r.width()),
                        f64::from(r.height()),
                    )
                })
                .collect();
            let output = labels
                .iter()
                .position(|(name, _)| name == "7.00")
                .expect("output value");
            assert!(
                bounds[output].1 + bounds[output].3 + 4.0 < 117.0,
                "Output must clear the orange/white separator"
            );
            for (i, (name, _)) in labels.iter().enumerate() {
                for j in i + 1..labels.len() {
                    assert!(
                        !overlaps(bounds[i], bounds[j], 4.0),
                        "Text clearance: {name} / {}",
                        labels[j].0
                    );
                }
                for c in CONTROLS {
                    let rect = if c.knob {
                        (
                            c.x - c.w - 11.0,
                            c.y - c.w - 11.0,
                            2.0 * (c.w + 11.0),
                            2.0 * (c.w + 11.0),
                        )
                    } else {
                        (c.x, c.y, c.w, c.h)
                    };
                    let a = bounds[i];
                    // A selector's own label belongs inside its button.
                    if !c.knob
                        && a.0 >= rect.0
                        && a.0 + a.2 <= rect.0 + rect.2
                        && a.1 >= rect.1
                        && a.1 + a.3 <= rect.1 + rect.3
                    {
                        continue;
                    }
                    assert!(
                        !overlaps(a, rect, 4.0),
                        "Control clearance: {name} / {}",
                        c.id
                    );
                }
            }
        }
    }
    #[test]
    // Regresyon senaryosu controls_cover_every_parameter_and_do_not_overlap: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: ModelFParams::default → CONTROLS.len → p.list → iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn controls_cover_every_parameter_and_do_not_overlap() {
        let p = ModelFParams::default();
        assert_eq!(
            CONTROLS.len(),
            p.list()
                .iter()
                .filter(|(id, _)| !id.starts_with("mseg_") && *id!="character")
                .count()
        );
        for c in CONTROLS {
            assert!(p.get(c.id).is_some());
            assert!(c.hit(c.x, c.y));
        }
        for c in CONTROLS {
            for other in CONTROLS {
                if c.id != other.id {
                    assert!(!other.hit(c.x, c.y), "{} overlaps {}", c.id, other.id);
                }
            }
        }
    }
    #[test]
    // Regresyon senaryosu panel_and_browser_render_at_two_scales: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn panel_and_browser_render_at_two_scales() {
        let p = ModelFParams::default();
        let mut v = View {
            name: "Seven Suns".into(),
            category: "Lead".into(),
            ..View::default()
        };
        assert!(render(&svg(&p, &v), W, H).is_ok());
        v.browser = true;
        v.rows = crate::presets::factory();
        assert!(render(&svg(&p, &v), W * 2, H * 2).is_ok());
    }
}

// SuperSaw'ın tek bankı ve kapalı FM'nin derinlik kontrolü bilinçli olarak pasiftir.
pub fn control_enabled(p: &ModelFParams, id: &str) -> bool {
    if id=="character_shape" {return matches!(p.character.value().round() as u8,2|3|5);}
    if p.character.value().round()==5.0 && matches!(id,"mode" | "waveform" | "osc2_waveform" | "osc2_octave" | "osc2_level" | "osc_sync" | "fm_on" | "fm_amount") {return false;}
    if p.mode.value().round() == 1.0 && matches!(id, "osc2_waveform" | "osc2_octave" | "osc2_level" | "osc_sync" | "fm_on" | "fm_amount") { return false; }
    id != "fm_amount" || p.fm_on.value() > 0.5
}

// Fazıl Irak — 25.10.2026: Output düğmesi de üst panelin kilidini izler.
pub fn control_enabled_for_view(p:&ModelFParams,v:&View,id:&str)->bool {
    control_enabled(p,id) && (!matches!(id,"volume" | "global_tuning" | "character_shape") || v.header_enabled())
}
// Satır ve silme hit-test'i render ile aynı boyutları kullanır.
pub fn user_delete_hit(v:&View,x:f64,y:f64)->Option<usize> {
    if !v.browser || v.filter!="User" || !(234.0..594.0).contains(&y) {return None;}
    let col=if (516.0..552.0).contains(&x) {0} else if (1036.0..1072.0).contains(&x) {1} else {return None;};
    let row=((y-234.)/36.) as usize;
    if (y-234.)%36.>=30. {return None;}
    let index=v.offset+col*10+row;
    v.rows.get(index).filter(|p|p.source_path.is_some()).map(|_|index)
}

#[cfg(test)]
mod tuning_gesture_tests {
    use super::*;
    #[test]
    fn tuning_hit_drag_direction_resolution_and_limits() {
        let p=ModelFParams::default();
        let c=CONTROLS.iter().find(|c|c.hit(120.0,710.0)).unwrap();
        assert_eq!(c.id,"global_tuning");assert!(c.draggable());
        let base=p.display_normalized(c.id);let hz=|n|p.global_tuning.preview_plain(n);
        assert!((hz(c.drag_normalized(base,20.0,false))-442.0).abs()<0.051);
        assert!((hz(c.drag_normalized(base,-20.0,false))-438.0).abs()<0.051);
        assert!((hz(c.drag_normalized(base,20.0,true))-440.2).abs()<0.051);
        assert_eq!(hz(c.drag_normalized(base,10000.0,false)),crate::params::GLOBAL_TUNING_MAX);
        assert_eq!(hz(c.drag_normalized(base,-10000.0,false)),400.0);
        assert!(control_enabled_for_view(&p,&View::default(),c.id));
        assert!(!control_enabled_for_view(&p,&View{envelope:true,..Default::default()},c.id));
    }
}

#[cfg(test)]
mod resonance_ui_tests {
    use super::*;
    #[test]
    fn caution_arc_is_character_specific_and_keeps_the_same_geometry() {
        let c=CONTROLS.iter().find(|c|c.id=="emphasis").unwrap();
        assert_eq!((c.x,c.y,c.w),(607.0,245.0,27.0));
        for (character,start) in [0.75,0.83,0.82,0.90,0.80,0.94].into_iter().enumerate() {
            let p=ModelFParams{character:FloatParam::new("Character",character as f32,FloatRange::Linear{min:0.0,max:5.0}),..Default::default()};
            let svg=svg(&p,&View::default());
            assert_eq!(svg.matches("data-role=\"emphasis-high-resonance\"").count(),1);
            assert!(svg.contains(&format!("data-start=\"{start:.2}\"")));
            let arc=svg.split("data-role=\"emphasis-high-resonance\"").nth(1).unwrap().split("/>").next().unwrap();
            assert!(arc.contains("A 27 27") && arc.contains("stroke-width=\"2\""));
        }
    }
    #[test]
    fn acid_emphasis_drag_and_wheel_are_finer_without_reinterpreting_the_parameter() {
        let c=CONTROLS.iter().find(|c|c.id=="emphasis").unwrap();
        assert!((c.drag_for_character(0.7,10.0,false,4)-0.72).abs()<1e-6);
        assert!((c.drag_for_character(0.7,10.0,false,0)-0.75).abs()<1e-6);
        assert!((c.drag_for_character(0.7,10.0,true,4)-0.702).abs()<1e-6);
        assert_eq!(c.wheel_step(false,4),0.005);assert_eq!(c.wheel_step(true,4),0.001);
        for mode in [0,1,2,3,5] {assert_eq!(c.wheel_step(false,mode),0.02);}
    }
}

#[cfg(test)]
mod tuning_quick_select_tests {
    use super::*;
    #[test]
    fn quick_select_cycles_both_ways_and_handles_manual_values() {
        for (i,hz) in GLOBAL_TUNING_CHOICES.into_iter().enumerate() {
            assert_eq!(next_global_tuning(hz,false),GLOBAL_TUNING_CHOICES[(i+1)%5]);
            assert_eq!(next_global_tuning(hz,true),GLOBAL_TUNING_CHOICES[(i+4)%5]);
        }
        assert_eq!(next_global_tuning(439.1,false),440.0);assert_eq!(next_global_tuning(439.1,true),432.0);
        assert_eq!(next_global_tuning(540.0,false),432.0);assert_eq!(next_global_tuning(400.0,true),528.0);
    }
    #[test]
    fn only_orange_strip_cycles_and_modal_windows_block_it() {
        let p=ModelFParams::default();let v=View::default();
        assert_eq!(global_tuning_cycle_at(&p,&v,204.0,710.0,false),Some(442.0));
        assert_eq!(global_tuning_cycle_at(&p,&v,204.0,710.0,true),Some(432.0));
        assert_eq!(global_tuning_cycle_at(&p,&v,190.0,710.0,false),None);
        assert_eq!(global_tuning_cycle_at(&p,&v,216.0,710.0,false),None);
        for v in [View{browser:true,..Default::default()},View{envelope:true,..Default::default()},View{save:true,..Default::default()},View{about:true,..Default::default()}] {
            assert_eq!(global_tuning_cycle_at(&p,&v,204.0,710.0,false),None);
        }
    }
    #[test]
    fn tuning_value_colours_and_extended_range_match_the_requested_values() {
        for (hz,colour) in [(432.0,"#71c88b"),(440.0,"#ef5350"),(442.0,"#ed6c46"),(444.0,"#ed6c46"),(528.0,"#69aefb"),(439.1,"#c8cbc6"),(432.1,"#c8cbc6")] {
            assert_eq!(global_tuning_colour(hz),colour);
            let p=ModelFParams{global_tuning:FloatParam::new("Global Tuning",hz,FloatRange::Linear{min:crate::params::GLOBAL_TUNING_MIN,max:crate::params::GLOBAL_TUNING_MAX}).with_step_size(0.1),..Default::default()};
            let svg=svg(&p,&View::default());assert!(svg.contains("data-role=\"global-tuning-cycle\""));
            assert!(svg.contains(&format!("fill=\"{colour}\" font-size=\"11\" text-anchor=\"end\">{hz:.1} Hz")));
            assert!((p.global_tuning.preview_plain(p.global_tuning.preview_normalized(hz))-hz).abs()<0.051);
        }
        let p=ModelFParams::default();let c=CONTROLS.iter().find(|c|c.id=="global_tuning").unwrap();
        let next=p.global_tuning.preview_plain(p.display_normalized(c.id)+c.wheel_step(false,0));
        assert!((next-440.1).abs()<0.051);
    }
}

