// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
//! Model F palette, URW Gothic typography and charcoal knobs.
use crate::{dsp, params::EqParams};
use nih_plug::prelude::*;
use std::fmt::Write;
pub const W: u32 = 800;
pub const H: u32 = 418;
// logo_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn logo_hit(x: f64, y: f64) -> bool {
    (28. ..=222.).contains(&x) && (20. ..=87.).contains(&y)
}
// about_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn about_hit(x: f64, y: f64) -> bool {
    (180. ..=620.).contains(&x) && (48. ..=314.).contains(&y)
}
// about_close_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn about_close_hit(x: f64, y: f64) -> bool {
    ((330. ..=470.).contains(&x) && (260. ..=292.).contains(&y))
        || ((582. ..=610.).contains(&x) && (56. ..=83.).contains(&y))
}
pub const KNOBS: [(f64, f64, f64); 9] = [
    (350., 238., 42.),
    (350., 238., 83.),
    (555., 175., 26.),
    (688., 175., 26.),
    (119., 175., 26.),
    (119., 305., 26.),
    (688., 305., 26.),
    (555., 305., 26.),
    (350., 238., 63.),
];
// frequency_limits alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn frequency_limits(i: usize) -> Option<(f32, f32)> {
    match i {
        1 => Some((200., 4000.)),
        4 => Some((20., 500.)),
        5 => Some((1000., 22000.)),
        _ => None,
    }
}
// frequency_position alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn frequency_position(value: f32, min: f32, max: f32) -> f32 {
    ((value.clamp(min, max) / min).ln() / (max / min).ln()).clamp(0., 1.)
}
/// Visual travel is logarithmic in Hz. Keep the existing host parameter
/// normalization intact so sessions and recorded automation retain their sound.
// knob_position alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn knob_position(p: &EqParams, i: usize) -> f32 {
    let param = p.knobs()[i];
    match frequency_limits(i) {
        Some((min, max)) => frequency_position(param.unmodulated_plain_value(), min, max),
        None => param.unmodulated_normalized_value(),
    }
}
// default_position alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn default_position(p: &EqParams, i: usize) -> f32 {
    let param = p.knobs()[i];
    match frequency_limits(i) {
        Some((min, max)) => frequency_position(param.default_plain_value(), min, max),
        None => param.default_normalized_value(),
    }
}
// host_normalized alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn host_normalized(p: &EqParams, i: usize, position: f32) -> f32 {
    let position = position.clamp(0., 1.);
    match frequency_limits(i) {
        Some((min, max)) => {
            let hz = if position == 0. {
                min
            } else if position == 1. {
                max
            } else {
                min * (max / min).powf(position)
            };
            p.knobs()[i].preview_normalized(hz)
        }
        None => position,
    }
}
// hit: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
pub fn hit(x: f64, y: f64) -> Option<usize> {
    let radius = (x - 350.).hypot(y - 238.);
    if radius <= 44. {
        return Some(0);
    }
    if radius <= 65. {
        return Some(8);
    }
    if radius < 93. {
        return Some(1);
    }
    KNOBS.iter().enumerate().find_map(|(i, &(cx, cy, r))| {
        (i >= 2 && i != 8 && (x - cx).hypot(y - cy) < r + 10.).then_some(i)
    })
}
// power_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn power_hit(x: f64, y: f64) -> bool {
    (552. ..=650.).contains(&x) && (24. ..=56.).contains(&y)
}
// analog_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn analog_hit(x: f64, y: f64) -> bool {
    (664. ..=772.).contains(&x) && (24. ..=56.).contains(&y)
}
// slope_hit alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn slope_hit(x: f64, y: f64) -> Option<(bool, usize)> {
    for (low, cy) in [(true, 226.), (false, 362.)] {
        if (cy..=cy + 21.).contains(&y) && (60. ..178.).contains(&x) {
            return Some((low, ((x - 60.) / 24.) as usize));
        }
    }
    None
}
// value alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn value(p: &EqParams, i: usize) -> String {
    let v = p.knobs()[i].value();
    match i {
        0 | 2 | 6 => format!("{v:+.1} dB"),
        1 => format!("{v:.0} Hz"),
        3 | 8 => format!("{v:.2}"),
        4 if v <= 20.01 => "OFF".into(),
        5 if v >= 21999. => "OFF".into(),
        7 => format!("{v:.0}%"),
        4 | 5 if v >= 1000. => format!("{:.2} kHz", v / 1000.),
        _ => format!("{v:.0} Hz"),
    }
}
// text alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn text(s: &mut String, x: f64, y: f64, size: u32, fill: &str, body: &str, anchor: &str) {
    let _ = write!(
        s,
        "<text x='{x}' y='{y}' text-anchor='{anchor}' font-size='{size}' fill='{fill}'>{body}</text>"
    );
}
// center alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn center(s: &mut String, x: f64, y: f64, size: u32, fill: &str, body: &str) {
    text(s, x, y, size, fill, body, "middle");
}
// pointer alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn pointer(s: &mut String, x: f64, y: f64, r: f64, t: f32) {
    let angle = (-225. + 270. * t as f64).to_radians();
    let _ = write!(
        s,
        "<path d='M {} {} L {} {}' stroke='#ed6c46' stroke-width='3' stroke-linecap='round'/>",
        x + angle.cos() * r * 0.62,
        y + angle.sin() * r * 0.62,
        x + angle.cos() * r * 0.89,
        y + angle.sin() * r * 0.89
    );
}
// ticks alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn ticks(s: &mut String, x: f64, y: f64, r: f64) {
    for j in 0..=20 {
        let a = (-225. + j as f64 * 270. / 20.).to_radians();
        let _ = write!(
            s,
            "<path d='M {} {} L {} {}' stroke='#525b5e' stroke-width='1.2'/>",
            x + a.cos() * (r + 5.),
            y + a.sin() * (r + 5.),
            x + a.cos() * (r + 9.),
            y + a.sin() * (r + 9.)
        );
    }
}
// knob alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn knob(s: &mut String, x: f64, y: f64, r: f64, t: f32) {
    ticks(s, x, y, r);
    let _ = write!(
        s,
        "<circle cx='{x}' cy='{y}' r='{r}' fill='#101415' stroke='#3e474a' stroke-width='2'/><circle cx='{x}' cy='{y}' r='{}' fill='#292f31'/>",
        r - 4.
    );
    pointer(s, x, y, r, t);
}
// main_knob alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: ticks → s.push_str → knob_position → to_radians. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn main_knob(s: &mut String, p: &EqParams) {
    ticks(s, 350., 238., 83.);
    // Flat Model F orange frequency ring, without lighting or drop shadows.
    s.push_str("<circle cx='350' cy='238' r='84' fill='#101415' stroke='#3e474a' stroke-width='2'/><circle cx='350' cy='238' r='80' fill='#ed6c46'/>");
    let a = (-225. + 270. * knob_position(p, 1) as f64).to_radians();
    let _ = write!(
        s,
        "<path d='M {} {} L {} {}' stroke='#101415' stroke-width='3.5' stroke-linecap='round'/>",
        350. + a.cos() * 70.,
        238. + a.sin() * 70.,
        350. + a.cos() * 76.,
        238. + a.sin() * 76.
    );
    // A separate flat ivory ring controls only the tilt shelves' Q.
    s.push_str("<circle cx='350' cy='238' r='67' fill='#101415'/><circle cx='350' cy='238' r='63' fill='#e5e7df'/>");
    let qa = (-225. + 270. * knob_position(p, 8) as f64).to_radians();
    let _ = write!(
        s,
        "<path d='M {} {} L {} {}' stroke='#292f31' stroke-width='3' stroke-linecap='round'/>",
        350. + qa.cos() * 51.,
        238. + qa.sin() * 51.,
        350. + qa.cos() * 59.,
        238. + qa.sin() * 59.
    );
    // Top-view fluted cap retains its own independent level control.
    s.push_str("<circle cx='350' cy='238' r='45' fill='#101415'/>");
    let rotation = (-225. + 270. * p.tilt.unmodulated_normalized_value() as f64).to_radians();
    let mut outline = String::new();
    for i in 0..=192 {
        let angle = i as f64 * std::f64::consts::TAU / 192.;
        let r = 40. + 1.8 * (12. * (angle - rotation)).cos();
        let _ = write!(
            outline,
            "{} {:.2} {:.2} ",
            if i == 0 { "M" } else { "L" },
            350. + r * angle.cos(),
            238. + r * angle.sin()
        );
    }
    outline.push('Z');
    let _ = write!(
        s,
        "<path d='{outline}' fill='#292f31' stroke='#3e474a' stroke-width='1.2'/>"
    );
    pointer(s, 350., 238., 40., p.tilt.unmodulated_normalized_value());
}
// button alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
fn button(s: &mut String, x: f64, y: f64, w: f64, label: &str, on: bool) {
    let _ = write!(
        s,
        "<rect x='{x}' y='{y}' width='{w}' height='32' rx='4' fill='{}' stroke='{}' stroke-width='1.3'/>",
        if on { "#ed6c46" } else { "#222729" },
        if on { "#ed6c46" } else { "#343b3e" }
    );
    center(
        s,
        x + w / 2.,
        y + 20.,
        11,
        if on { "#101415" } else { "#c3cbc7" },
        label,
    );
}
// graph alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: let → spectrum.iter → enumerate → clamp. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn graph(
    s: &mut String,
    x: f64,
    y: f64,
    p: &EqParams,
    rate: f64,
    spectrum: &[f32; crate::spectrum::BINS],
) {
    let (w, h) = (240., 54.);
    let _ = write!(
        s,
        "<rect x='{x}' y='{y}' width='{w}' height='{h}' rx='4' fill='#141819' stroke='#354043'/>"
    );
    let mut area = format!("M {x} {} ", y + h - 1.);
    for (i, db) in spectrum.iter().enumerate() {
        let px = x + (i as f64 + 0.5) * w / crate::spectrum::BINS as f64;
        let py = y + 1. + (1. - ((*db as f64 + 90.) / 90.).clamp(0., 1.)) * (h - 2.);
        let _ = write!(area, "L {px:.1} {py:.1} ");
    }
    let _ = write!(area, "L {} {} Z", x + w, y + h - 1.);
    let _ = write!(
        s,
        "<path d='{area}' fill='#a1b9ad' fill-opacity='.17' stroke='#94b3a2' stroke-opacity='.55' stroke-width='.8'/>"
    );
    for i in 1..5 {
        let gx = x + w * i as f64 / 5.;
        let _ = write!(s, "<path d='M {gx} {y} v {h}' stroke='#283235'/>");
    }
    for i in 1..4 {
        let gy = y + h * i as f64 / 4.;
        let _ = write!(s, "<path d='M {x} {gy} h {w}' stroke='#283235'/>");
    }
    let _ = write!(
        s,
        "<path d='M {x} {} h {w}' stroke='#596265' stroke-dasharray='3 4'/>",
        y + h / 2.
    );
    let mut settings = p.settings();
    settings.volume = 0.;
    let mut path = String::new();
    for i in 0..=160 {
        let f = 20. * 1000f64.powf(i as f64 / 160.);
        let db = if p.bypass.value() {
            0.
        } else {
            dsp::response(settings, f.min(rate * 0.49), rate)
        };
        let gy = (y + h / 2. - db * h / 48.).clamp(y + 2., y + h - 2.);
        let _ = write!(
            path,
            "{}{:.2},{:.2} ",
            if i == 0 { "M" } else { "L" },
            x + w * i as f64 / 160.,
            gy
        );
    }
    let _ = write!(
        s,
        "<path d='{path}' stroke='#ed6c46' fill='none' stroke-width='1.8'/>"
    );
    text(s, x, y + h + 12., 7, "#899598", "20 Hz", "start");
    center(s, x + w * 0.566, y + h + 12., 7, "#899598", "1 kHz");
    text(s, x + w, y + h + 12., 7, "#899598", "20 kHz", "end");
}
// svg alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn svg(p: &EqParams, meter: f32, rate: f64) -> String {
    svg_with_spectrum(p, meter, rate, &[-100.; crate::spectrum::BINS])
}
// svg_with_spectrum alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: text → graph → button → p.bypass.value. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn svg_with_spectrum(
    p: &EqParams,
    meter: f32,
    rate: f64,
    spectrum: &[f32; crate::spectrum::BINS],
) -> String {
    let mut s = format!(
        "<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{H}' viewBox='0 0 {W} {H}'><rect width='{W}' height='{H}' rx='12' fill='#141819'/><g font-family='URW Gothic, sans-serif'>"
    );
    text(&mut s, 28., 59., 40, "#e5e7df", "F EQ", "start");
    text(
        &mut s,
        30.,
        81.,
        9,
        "#8d9799",
        "FAZIL IRAK / TILT EQUALIZER",
        "start",
    );
    graph(&mut s, 245., 24., p, rate, spectrum);
    button(
        &mut s,
        552.,
        24.,
        98.,
        if p.bypass.value() { "BYPASS" } else { "EQ ON" },
        !p.bypass.value(),
    );
    button(&mut s, 664., 24., 108., "ANALOG", p.analog.value());
    let clip = meter >= 1.;
    let _ = write!(
        s,
        "<circle cx='764' cy='77' r='3' fill='{}'/>",
        if clip {
            "#ed6c46"
        } else if meter > 0.004 {
            "#8faa95"
        } else {
            "#313b3d"
        }
    );
    text(
        &mut s,
        748.,
        80.,
        8,
        "#8d9799",
        if clip { "CLIP" } else { "OUTPUT" },
        "end",
    );
    s.push_str("<path d='M 28 100 H 772' stroke='#ed6c46' stroke-width='2'/><path d='M 28 105 H 772' stroke='#dce0d8' stroke-width='2'/><rect x='28' y='117' width='744' height='275' rx='8' fill='#1b2123' stroke='#343c3f'/><path d='M 230 136 V 373 M 478 136 V 373' stroke='#30383a'/>");
    main_knob(&mut s, p);
    center(&mut s, 350., 263., 13, "#ed6c46", &value(p, 0));
    center(&mut s, 317., 354., 13, "#ed6c46", &value(p, 1));
    center(
        &mut s,
        404.,
        354.,
        12,
        "#e5e7df",
        &format!("Q {}", value(p, 8)),
    );
    center(&mut s, 272., 331., 8, "#c3cbc7", "DARKER");
    center(&mut s, 428., 331., 8, "#c3cbc7", "BRIGHTER");
    center(
        &mut s,
        350.,
        375.,
        7,
        "#899598",
        "INNER: LEVEL / MIDDLE: Q / OUTER: FREQ",
    );
    for (i, label, sub) in [
        (4, "LOW CUT", ""),
        (5, "HIGH CUT", ""),
        (2, "BASS LIFT", "300 Hz"),
        (3, "BASS Q", "300 Hz SHELF"),
        (7, "SATURATION", "HARMONICS"),
        (6, "VOLUME", "OUTPUT GAIN"),
    ] {
        let (x, y, r) = KNOBS[i];
        center(&mut s, x, y - 40., 10, "#afb8b8", label);
        knob(&mut s, x, y, r, knob_position(p, i));
        center(&mut s, x, y + 44., 11, "#ed6c46", &value(p, i));
        if i == 4 || i == 5 {
            let selected = if i == 4 {
                p.low_slope.value()
            } else {
                p.high_slope.value()
            } as usize;
            let row = y + if i == 4 { 51. } else { 57. };
            for (j, n) in crate::cuts::SLOPES.iter().enumerate() {
                let left = x - 59. + j as f64 * 24.;
                let _ = write!(
                    s,
                    "<rect x='{left}' y='{row}' width='22' height='21' rx='3' fill='{}' stroke='{}'/>",
                    if j == selected { "#ed6c46" } else { "#222729" },
                    if j == selected { "#ed6c46" } else { "#343b3e" }
                );
                center(
                    &mut s,
                    left + 11.,
                    row + 14.,
                    9,
                    if j == selected { "#101415" } else { "#afb8b8" },
                    &n.to_string(),
                );
            }
        } else {
            center(&mut s, x, y + 64., 8, "#899598", sub);
        }
    }
    text(
        &mut s,
        30.,
        409.,
        8,
        "#8d9799",
        "DRAG / SCROLL · SHIFT: FINE · CTRL-CLICK: RESET",
        "start",
    );
    text(&mut s, 770., 409., 8, "#8d9799", "F EQ / 0.1.10", "end");
    s.push_str("</g></svg>");
    s
}
// options alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
pub fn options() -> resvg::usvg::Options<'static> {
    let mut db = resvg::usvg::fontdb::Database::new();
    db.load_font_data(include_bytes!("../third_party/urw-gothic/URWGothic-Book.otf").to_vec());
    resvg::usvg::Options {
        fontdb: std::sync::Arc::new(db),
        ..Default::default()
    }
}
// with_about alt adımı: Kontrol geometrisi, fiziksel değer gösterimi, SVG/piksel çizimi ve kullanıcı hit-test haritasıdır.
// Gövdedeki başlıca çağrılar: svg.strip_suffix → unwrap_or → to_owned → s.push_str. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn with_about(svg: &str) -> String {
    let mut s = svg.strip_suffix("</g></svg>").unwrap_or(svg).to_owned();
    s.push_str("<rect width='800' height='418' fill='#070b0c' opacity='.86'/><rect x='180' y='48' width='440' height='266' rx='10' fill='#22292b' stroke='#566265' stroke-width='1.2'/>");
    center(&mut s, 400., 78., 9, "#ed6c46", "ABOUT");
    center(&mut s, 400., 121., 38, "#e5e7df", "F EQ");
    center(
        &mut s,
        400.,
        145.,
        11,
        "#afb8b8",
        &format!("Version {}", env!("CARGO_PKG_VERSION")),
    );
    center(&mut s, 400., 176., 17, "#ed6c46", "FAZIL IRAK");
    s.push_str("<path d='M 212 194 H 588' stroke='#465154'/><path d='M 590 64 L 600 74 M 600 64 L 590 74' stroke='#afb8b8' stroke-width='1.5'/>");
    center(
        &mut s,
        400.,
        216.,
        12,
        "#dce0d8",
        "Tilt equalizer / Analog colour / Saturation",
    );
    button(&mut s, 330., 260., 140., "CLOSE", true);
    center(&mut s, 400., 304., 7, "#8d9799", "ESC / ENTER TO CLOSE");
    s.push_str("</g></svg>");
    s
}
// Hazırlanmış durumdan ses veya panel çıktısını üretir; bu dosyanın DSP/GUI bağlamını aşağıdaki gövdeden ayırın.
pub fn render(
    svg: &str,
    width: u32,
    height: u32,
    options: &resvg::usvg::Options,
) -> Option<resvg::tiny_skia::Pixmap> {
    let tree = resvg::usvg::Tree::from_str(svg, options).ok()?;
    let mut pixels = resvg::tiny_skia::Pixmap::new(width, height)?;
    resvg::render(
        &tree,
        resvg::tiny_skia::Transform::from_scale(width as f32 / W as f32, height as f32 / H as f32),
        &mut pixels.as_mut(),
    );
    Some(pixels)
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    // Regresyon senaryosu frequency_travel_matches_endpoints_and_actual_hz: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: EqParams::default → let → frequency_limits → unwrap. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn frequency_travel_matches_endpoints_and_actual_hz() {
        let p = EqParams::default();
        for i in [1, 4, 5] {
            let (min, max) = frequency_limits(i).unwrap();
            assert_eq!(frequency_position(min, min, max), 0.);
            assert_eq!(frequency_position(max, min, max), 1.);
            assert_eq!(p.knobs()[i].preview_plain(host_normalized(&p, i, 0.)), min);
            assert_eq!(p.knobs()[i].preview_plain(host_normalized(&p, i, 1.)), max);
            for position in [0.001, 0.02, 0.1, 0.25, 0.5, 0.9, 0.999] {
                let hz = p.knobs()[i].preview_plain(host_normalized(&p, i, position));
                assert!((frequency_position(hz, min, max) - position).abs() < 0.00001);
            }
            let middle = p.knobs()[i].preview_plain(host_normalized(&p, i, 0.5));
            assert!((middle / (min * max).sqrt() - 1.).abs() < 1e-6);
        }
        // Regression: the old 10% host value is 200.38 Hz, rounded to 200 Hz.
        // It must be drawn within a fraction of a degree of the minimum tick,
        // rather than 27 degrees away (two full tick intervals).
        let hz = p.pivot.preview_plain(0.1);
        assert!(hz > 200. && hz < 200.5);
        assert!(frequency_position(hz, 200., 4000.) * 270. < 0.2);
    }
    #[test]
    // Regresyon senaryosu concentric_controls_are_independent_hit_regions: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: hit → angle.to_radians → a.cos → a.sin. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn concentric_controls_are_independent_hit_regions() {
        assert_eq!(hit(350., 238.), Some(0));
        assert_eq!(hit(350., 194.), Some(0));
        assert_eq!(hit(425., 238.), Some(1));
        assert_eq!(hit(350., 164.), Some(1));
        assert_eq!(hit(350., 354.), None);
        assert_eq!(hit(119., 175.), Some(4));
        for angle in [-225f64, -135., -90., 0., 45.] {
            let a = angle.to_radians();
            assert_eq!(hit(350. + 30. * a.cos(), 238. + 30. * a.sin()), Some(0));
            assert_eq!(hit(350. + 54. * a.cos(), 238. + 54. * a.sin()), Some(8));
            assert_eq!(hit(350. + 75. * a.cos(), 238. + 75. * a.sin()), Some(1));
        }
    }
    #[test]
    // Regresyon senaryosu both_slope_rows_are_independently_selectable: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn both_slope_rows_are_independently_selectable() {
        for (low, y) in [(true, 242.), (false, 372.)] {
            for i in 0..5 {
                assert_eq!(slope_hit(71. + i as f64 * 24., y), Some((low, i)));
            }
        }
        assert_eq!(slope_hit(555., 250.), None);
    }
}
