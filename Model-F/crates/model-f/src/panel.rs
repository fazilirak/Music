// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
//! Shared vector panel and hit regions: the plug-in and preview use this same drawing.
use crate::{params::ModelFParams, presets::Preset};
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
    // hit: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    pub fn hit(&self, x: f64, y: f64) -> bool {
        if self.knob {
            (x - self.x).hypot(y - self.y) < self.w + 16.0
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
    k!("volume", "OUTPUT", 1045, 59, 22),
    b!("mode", "VOICE", 48, 194, 286, 32),
    b!("waveform", "WAVEFORM", 48, 258, 160, 34),
    b!("octave", "RANGE", 224, 258, 110, 34),
    k!("detune", "DETUNE", 115, 367, 36),
    k!("cutoff", "CUTOFF", 453, 245, 35),
    k!("emphasis", "EMPHASIS", 607, 245, 27),
    k!("contour", "CONTOUR", 607, 371, 24),
    k!("overload", "OVERLOAD", 453, 371, 24),
    b!("filter_mod", "FILTER MOD", 548, 146, 108, 28),
    k!("attack", "ATTACK", 756, 235, 30),
    k!("decay", "DECAY", 882, 235, 30),
    k!("sustain", "SUSTAIN", 1008, 235, 30),
    k!("noise", "NOISE", 756, 361, 27),
    b!("noise_color", "COLOR", 811, 343, 80, 32),
    b!("release", "CONTOUR DECAY", 924, 343, 142, 32),
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
        11,
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
    let v = p.get(id).map_or(0.0, |p| p.unmodulated_plain_value());
    match id {
        "mode" => MODES[(v.round() as usize).min(6)].into(),
        "waveform" => {
            if p.mode.value().round() == 1.0 {
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
    text(
        &mut s,
        350.0,
        96.0,
        10,
        "#ed6c46",
        &format!("{}  /  160 FACTORY SOUNDS", v.category.to_uppercase()),
        "start",
    );
    for (x, label) in [(782.0, "‹"), (820.0, "›")] {
        button(&mut s, x, 40.0, 30.0, 38.0, label, false);
    }
    for (x, label) in [(868.0, "SAVE"), (934.0, "LOAD")] {
        button(&mut s, x, 40.0, 56.0, 38.0, label, false);
    }
    text(&mut s, 871.0, 96.0, 9, "#8d9799", "EXPORT .MODELF", "start");
    s.push_str(r##"<path d="M28 118H1092" stroke="#ed6c46" stroke-width="2"/><path d="M28 123H1092" stroke="#dce0d8" stroke-width="2"/>"##);
    section(&mut s, 28.0, 132.0, 326.0, 310.0, "01", "OSCILLATOR");
    section(&mut s, 374.0, 132.0, 300.0, 310.0, "02", "LADDER FILTER");
    section(&mut s, 694.0, 132.0, 398.0, 310.0, "03", "CONTOUR / MIXER");
    section(&mut s, 28.0, 464.0, 326.0, 202.0, "04", "MODULATION");
    section(&mut s, 374.0, 464.0, 718.0, 210.0, "05", "ARPEGGIATOR");
    for (i, c) in CONTROLS.iter().enumerate() {
        let param = p.get(c.id).expect("panel control exists");
        let n = f64::from(param.unmodulated_normalized_value());
        let active = v.hover == Some(i);
        let accent = if active { "#ff956f" } else { "#ed6c46" };
        if c.knob {
            let r = c.w;
            for t in 0..=10 {
                let a = (-225.0 + 27.0 * f64::from(t)).to_radians();
                let (sn, cs) = a.sin_cos();
                let _ = write!(
                    s,
                    r###"<path d="M {} {} L {} {}" stroke="#525b5e" stroke-width="1.4"/>"###,
                    c.x + cs * (r + 7.0),
                    c.y + sn * (r + 7.0),
                    c.x + cs * (r + 11.0),
                    c.y + sn * (r + 11.0)
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
                "release" | "filter_mod" | "osc_mod" | "arp_on" | "arp_latch" | "arp_sync"
            );
            let label = if switch {
                format!("{}  {}", c.label, value(p, c.id))
            } else {
                value(p, c.id)
            };
            button(&mut s, c.x, c.y, c.w, c.h, &label, switch && n > 0.5);
            if !switch {
                text(&mut s, c.x, c.y - 9.0, 9, "#899598", c.label, "start");
            }
        }
    }
    // The lamp follows the return saturation stage's input drive, not output volume.
    let lamp = v.overload.clamp(0.0, 1.0);
    let _ = write!(
        s,
        r##"<circle cx="506" cy="349" r="10" fill="#ed6c46" opacity="{}"/><circle cx="506" cy="349" r="6" fill="#43271e" stroke="#644235"/><circle cx="506" cy="349" r="5" fill="#ff794b" opacity="{}"/>"##,
        lamp * 0.22,
        lamp
    );
    let super_mode = p.mode.value().round() == 1.0;
    text(
        &mut s,
        189.0,
        347.0,
        15,
        "#dce0d8",
        if super_mode { "7 × SAW" } else { "2 × VCO" },
        "start",
    );
    text(
        &mut s,
        189.0,
        370.0,
        9,
        "#8c999b",
        if super_mode {
            "JP DETUNE CURVE"
        } else {
            "Model F CIRCUIT CORE"
        },
        "start",
    );
    text(
        &mut s,
        189.0,
        393.0,
        9,
        "#8c999b",
        if super_mode {
            "8 NOTE POLYPHONY"
        } else {
            "ONE LADDER. BIG SOUND."
        },
        "start",
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
        709.0,
        9,
        "#899598",
        "PLAY A–K  /  W E T Y U = BLACK KEYS",
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
    button(
        &mut s,
        x,
        y,
        w,
        h,
        "MULTI-STAGE ENV.",
        p.mseg_enable.value() > 0.5,
    );
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
                504.0,
                30.0,
                &format!("{:02}   {}", v.offset + i + 1, preset.name),
                preset.name == v.name,
            );
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
            "NAME  /  TYPE TO EDIT",
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
        text(
            &mut s,
            560.0,
            446.0,
            11,
            "#9aabae",
            "Seven-oscillator SuperSaw · Arpeggiator · 120 presets",
            "middle",
        );
        let (bx, by, bw, bh) = ABOUT_CLOSE;
        button(&mut s, bx, by, bw, bh, "CLOSE", true);
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
        assert!(drawing.contains("FAZIL IRAK") && drawing.contains("VERSION 0.1.9"));
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
                .filter(|(id, _)| !id.starts_with("mseg_"))
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
