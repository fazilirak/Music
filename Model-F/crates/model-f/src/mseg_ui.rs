// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
//! Model F's envelope overlay. Hit testing and drawing share the same coordinates.
use crate::{
    mseg::{self, CAP, Config, Shape},
    panel::{self, Control},
    params::ModelFParams,
};
use keyboard_types::{Key, Modifiers};
use nih_plug::prelude::*;
use std::{
    fmt::Write,
    time::{Duration, Instant},
};
pub const OPEN: (f64, f64, f64, f64) = (236., 687., 170., 43.);
pub const PLOT: (f64, f64, f64, f64) = (56., 244., 1008., 240.);
pub const RECT: (f64, f64, f64, f64) = (28., 132., 1064., 542.);
// AD/Multi ortak alanları Amount, Target ve mod düğmesidir; geri kalanlar karşılıklı kilitlenir.
pub fn enabled(p: &ModelFParams, id: &str) -> bool {
    match id {
        "mseg_enable" | "mseg_amount" | "mseg_target" => true,
        "mseg_attack" | "mseg_decay" => p.mseg_enable.value() <= 0.5,
        _ => p.mseg_enable.value() > 0.5,
    }
}
pub const CONTROLS: &[Control] = &[
    Control {
        id: "mseg_duration",
        label: "DURATION",
        x: 100.,
        y: 558.,
        w: 25.,
        h: 0.,
        knob: true,
    },
    Control {
        id: "mseg_curve",
        label: "CURVE",
        x: 230.,
        y: 558.,
        w: 25.,
        h: 0.,
        knob: true,
    },
    Control {
        id: "mseg_mode",
        label: "PLAYBACK",
        x: 304.,
        y: 543.,
        w: 112.,
        h: 32.,
        knob: false,
    },
    Control {
        id: "mseg_attack",
        label: "AD ATTACK",
        x: 480.,
        y: 558.,
        w: 25.,
        h: 0.,
        knob: true,
    },
    Control {
        id: "mseg_decay",
        label: "AD DECAY",
        x: 610.,
        y: 558.,
        w: 25.,
        h: 0.,
        knob: true,
    },
    Control {
        id: "mseg_amount",
        label: "AMOUNT",
        x: 740.,
        y: 558.,
        w: 25.,
        h: 0.,
        knob: true,
    },
    Control {
        id: "mseg_target",
        label: "TARGET",
        x: 940.,
        y: 543.,
        w: 124.,
        h: 32.,
        knob: false,
    },
    Control {
        id: "mseg_gridx",
        label: "X GRID",
        x: 56.,
        y: 190.,
        w: 88.,
        h: 30.,
        knob: false,
    },
    Control {
        id: "mseg_gridy",
        label: "Y GRID",
        x: 156.,
        y: 190.,
        w: 88.,
        h: 30.,
        knob: false,
    },
    Control {
        id: "mseg_snap",
        label: "SNAP",
        x: 256.,
        y: 190.,
        w: 88.,
        h: 30.,
        knob: false,
    },
    Control {
        id: "mseg_brush",
        label: "BRUSH",
        x: 356.,
        y: 190.,
        w: 140.,
        h: 30.,
        knob: false,
    },
    Control {
        id: "mseg_enable",
        label: "MULTI",
        x: 536.,
        y: 144.,
        w: 128.,
        h: 30.,
        knob: false,
    },
];
#[derive(Debug, Clone)]
pub enum Drag {
    Node(usize),
    Handle(usize, bool),
    Knob(String, f64, f32),
    Brush(Option<(usize, f64)>),
}
#[derive(Debug, Default)]
pub struct State {
    pub selected: usize,
    pub drag: Option<Drag>,
    pub gestures: Vec<String>,
    last: Option<(Instant, f64, f64)>,
}
pub type Edits = Vec<(String, f32)>;
#[derive(Debug, Default, PartialEq)]
pub enum Command {
    #[default]
    None,
    Close,
    Save,
    Load,
}
// point_id alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
pub fn point_id(i: usize, s: &str) -> String {
    format!("mseg_p{i}_{s}")
}
// node_id alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
pub fn node_id(i: usize, s: &str) -> String {
    format!("mseg_n{i}_{s}")
}
// value alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
pub fn value(p: &ModelFParams, id: &str) -> f64 {
    f64::from(p.get(id).unwrap().value())
}
// xy alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
fn xy(x: f64, y: f64) -> (f64, f64) {
    ((x - PLOT.0) / PLOT.2, 1. - (y - PLOT.1) / PLOT.3)
}
// pixel alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
pub fn pixel(x: f64, y: f64) -> (f64, f64) {
    (PLOT.0 + x * PLOT.2, PLOT.1 + (1. - y) * PLOT.3)
}
// snap alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
fn snap(p: &ModelFParams, x: f64, y: f64) -> (f64, f64) {
    let (mut x, mut y) = (x.clamp(0., 1.), y.clamp(0., 1.));
    if p.mseg_snap.value() > 0.5 {
        let gx = 2usize.pow(p.mseg_gridx.value() as u32) as f64;
        let gy = 2usize.pow(p.mseg_gridy.value() as u32) as f64;
        x = (x * gx).round() / gx;
        y = (y * gy).round() / gy;
    }
    (x, y)
}
// curve_handle alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
// Gövdedeki başlıca çağrılar: iter → position → index.checked_sub → let. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn curve_handle(shape: &Shape, id: usize, incoming: bool) -> Option<(f64, f64, usize)> {
    let index = shape.nodes[..shape.count].iter().position(|n| n.id == id)?;
    let k = if incoming {
        index.checked_sub(1)?
    } else {
        index
    };
    if k + 1 >= shape.count {
        return None;
    }
    let (a, b) = (shape.nodes[k], shape.nodes[k + 1]);
    let delta = if (b.y - a.y).abs() > 1e-6 || a.bend.wave == 0 {
        b.y - a.y
    } else {
        a.bend.depth.min(1. - a.y).max(0.05)
    };
    let bend = if incoming {
        b.bend.incoming
    } else {
        a.bend.outgoing
    };
    let t = if incoming { 2. / 3. } else { 1. / 3. };
    Some((
        a.x + (b.x - a.x) * t,
        (a.y + delta * mseg::fraction(bend, incoming)).clamp(0., 1.),
        k,
    ))
}
impl State {
    // move_node alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
    // Gövdedeki başlıca çağrılar: let → snap → into → Config::from_params. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn move_node(&self, p: &ModelFParams, id: usize, x: f64, y: f64) -> Edits {
        let (x, y) = snap(p, x, y);
        if id == 0 || id == CAP + 1 {
            return vec![("mseg_start".into(), y as f32)];
        }
        let shape = Config::from_params(p).shape();
        let Some(i) = shape.nodes[..shape.count].iter().position(|n| n.id == id) else {
            return vec![];
        };
        let lo = shape.nodes[i - 1].x + 0.001;
        let hi = shape.nodes[i + 1].x - 0.001;
        let x = if p.mseg_snap.value() > 0.5 {
            let gx = 2usize.pow(p.mseg_gridx.value() as u32) as f64;
            let first = (lo * gx - 1e-5).ceil();
            let last = (hi * gx + 1e-5).floor();
            if first <= last {
                (x * gx).round().clamp(first, last) / gx
            } else {
                shape.nodes[i].x
            }
        } else {
            x.clamp(lo, hi.max(lo))
        };
        vec![
            (point_id(id - 1, "x"), x as f32),
            (point_id(id - 1, "y"), y as f32),
        ]
    }
    // down: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: contains → self.last.is_some_and → t.elapsed → Duration::from_millis. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn down(
        &mut self,
        p: &ModelFParams,
        x: f64,
        y: f64,
        right: bool,
        mods: Modifiers,
    ) -> (Edits, Command) {
        if x > 1000. && y < 177. {
            return (vec![], Command::Close);
        }
        if p.mseg_enable.value() > 0.5 && (144.0..174.0).contains(&y) {
            if (812.0..882.0).contains(&x) {
                return (vec![], Command::Save);
            }
            if (894.0..964.0).contains(&x) {
                return (vec![], Command::Load);
            }
        }
        let double = self.last.is_some_and(|(t, a, b)| {
            t.elapsed() < Duration::from_millis(350) && (x - a).hypot(y - b) < 9.
        });
        self.last = Some((Instant::now(), x, y));
        let c = Config::from_params(p);
        let shape = c.shape();
        if panel::in_rect(PLOT, x, y) {
            if !c.enabled { return (vec![], Command::None); }
            if mods.contains(Modifiers::CONTROL) && !right {
                self.drag = Some(Drag::Brush(None));
                return (self.motion(p, x, y, mods), Command::None);
            }
            for incoming in [true, false] {
                if let Some((a, b, _)) = curve_handle(&shape, self.selected, incoming) {
                    let (a, b) = pixel(a, b);
                    if (x - a).hypot(y - b) < 10. {
                        let id = node_id(self.selected, if incoming { "in" } else { "out" });
                        if double {
                            return (vec![(id, 0.)], Command::None);
                        }
                        self.drag = Some(Drag::Handle(self.selected, incoming));
                        return (vec![], Command::None);
                    }
                }
            }
            if let Some(n) = shape.nodes[..shape.count].iter().find(|n| {
                let (a, b) = pixel(n.x, n.y);
                (x - a).hypot(y - b) < 11.
            }) {
                self.selected = n.id;
                if double && n.id > 0 && n.id <= CAP {
                    return (vec![(point_id(n.id - 1, "on"), 0.)], Command::None);
                }
                self.drag = Some(Drag::Node(n.id));
                return (vec![], Command::None);
            }
            if double {
                let (a, b) = xy(x, y);
                let (a, b) = snap(p, a, b);
                if a > 0.
                    && a < 1.
                    && shape.nodes[..shape.count]
                        .iter()
                        .all(|n| (n.x - a).abs() >= 0.002)
                {
                    if let Some(i) = c.points.iter().position(|p| !p.on) {
                        self.selected = i + 1;
                        return (
                            vec![
                                (point_id(i, "x"), a as f32),
                                (point_id(i, "y"), b as f32),
                                (point_id(i, "on"), 1.),
                                (node_id(i + 1, "in"), 0.),
                                (node_id(i + 1, "out"), 0.),
                                (node_id(i + 1, "wave"), 0.),
                                (node_id(i + 1, "depth"), 1.),
                            ],
                            Command::None,
                        );
                    }
                }
            }
            return (vec![], Command::None);
        }
        if c.enabled && (190.0..220.0).contains(&y) && (536.0..726.0).contains(&x) {
            let id = node_id(self.selected, "wave");
            let v = value(p, &id);
            return (
                vec![(id, ((v as usize + if right { 8 } else { 1 }) % 9) as f32)],
                Command::None,
            );
        }
        if c.enabled && (x - 870.).hypot(y - 558.) < 40. {
            let id = node_id(self.selected, "depth");
            self.drag = Some(Drag::Knob(
                id.clone(),
                y,
                p.get(&id).unwrap().unmodulated_normalized_value(),
            ));
            return (vec![], Command::None);
        }
        for ctl in CONTROLS {
            if enabled(p, ctl.id) && ctl.hit(x, y) {
                let param = p.get(ctl.id).unwrap();
                if mods.intersects(Modifiers::CONTROL | Modifiers::ALT) {
                    return (
                        vec![(ctl.id.into(), param.default_plain_value())],
                        Command::None,
                    );
                }
                if ctl.knob && !right {
                    self.drag = Some(Drag::Knob(
                        ctl.id.into(),
                        y,
                        param.unmodulated_normalized_value(),
                    ));
                    return (vec![], Command::None);
                }
                let lo = param.preview_plain(0.);
                let hi = param.preview_plain(1.);
                let mut v = param.value() + if right { -1. } else { 1. };
                if v > hi {
                    v = lo;
                }
                if v < lo {
                    v = hi;
                }
                return (vec![(ctl.id.into(), v)], Command::None);
            }
        }
        (vec![], Command::None)
    }
    // motion alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
    // Gövdedeki başlıca çağrılar: self.drag.clone → Drag::Node → let → xy. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn motion(&mut self, p: &ModelFParams, x: f64, y: f64, mods: Modifiers) -> Edits {
        let allowed = match &self.drag {
            Some(Drag::Knob(id, ..)) => enabled(p, id),
            Some(_) => p.mseg_enable.value() > 0.5, _ => true,
        };
        if !allowed { self.drag = None; return vec![]; }
        match self.drag.clone() {
            Some(Drag::Node(id)) => {
                let (a, b) = xy(x, y);
                self.move_node(p, id, a, b)
            }
            Some(Drag::Handle(id, incoming)) => {
                let shape = Config::from_params(p).shape();
                if let Some((_, _, k)) = curve_handle(&shape, id, incoming) {
                    let (a, b) = (shape.nodes[k], shape.nodes[k + 1]);
                    let delta = if (b.y - a.y).abs() > 1e-6 || a.bend.wave == 0 {
                        b.y - a.y
                    } else {
                        a.bend.depth.min(1. - a.y).max(0.05)
                    };
                    if delta.abs() > 1e-6 {
                        let (_, v) = xy(x, y);
                        return vec![(
                            node_id(id, if incoming { "in" } else { "out" }),
                            mseg::bend_from_fraction((v - a.y) / delta, incoming) as f32,
                        )];
                    }
                }
                vec![]
            }
            Some(Drag::Knob(id, base_y, base)) => {
                let param = p.get(&id).unwrap();
                let n = base
                    + ((base_y - y)
                        * if mods.contains(Modifiers::SHIFT) {
                            0.0005
                        } else {
                            0.005
                        }) as f32;
                vec![(id, param.preview_plain(n.clamp(0., 1.)))]
            }
            Some(Drag::Brush(last)) => {
                let (a, b) = xy(x, y);
                let (a, b) = snap(p, a, b);
                let cells = 2usize.pow(p.mseg_gridx.value() as u32);
                let cell = ((a * cells as f64) as usize).min(cells - 1);
                let (prev, depth) = last.unwrap_or((cell, b));
                let mut config = Config::from_params(p);
                let mut edits = vec![];
                for n in prev.min(cell)..=prev.max(cell) {
                    let f = if cell == prev {
                        1.
                    } else {
                        (n as f64 - prev as f64) / (cell as f64 - prev as f64)
                    };
                    let d = depth + (b - depth) * f;
                    edits.extend(paint_cell(
                        &mut config,
                        n,
                        cells,
                        d,
                        p.mseg_brush.value() as usize,
                    ));
                }
                self.drag = Some(Drag::Brush(Some((cell, b))));
                edits
            }
            _ => vec![],
        }
    }
    // key: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: point_id → Config::from_params → shape → iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    pub fn key(&self, p: &ModelFParams, key: &Key, mods: Modifiers) -> Edits {
        if p.mseg_enable.value() <= 0.5 { return vec![]; }
        if matches!(key, Key::Delete | Key::Backspace) && self.selected > 0 && self.selected <= CAP
        {
            return vec![(point_id(self.selected - 1, "on"), 0.)];
        }
        let shape = Config::from_params(p).shape();
        if let Some(n) = shape.nodes[..shape.count]
            .iter()
            .find(|n| n.id == self.selected)
        {
            let step = if mods.contains(Modifiers::SHIFT) {
                0.001
            } else {
                0.01
            };
            let (mut x, mut y) = (n.x, n.y);
            match key {
                Key::ArrowLeft => x -= step,
                Key::ArrowRight => x += step,
                Key::ArrowUp => y += step,
                Key::ArrowDown => y -= step,
                _ => return vec![],
            };
            return self.move_node(p, n.id, x, y);
        }
        vec![]
    }
}
// paint_cell alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
// Gövdedeki başlıca çağrılar: iter → position → abs → next.points.iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn paint_cell(c: &mut Config, cell: usize, cells: usize, depth: f64, kind: usize) -> Edits {
    let mut next = *c;
    let l = cell as f64 / cells as f64;
    let r = (cell + 1) as f64 / cells as f64;
    for p in &mut next.points {
        if p.on && p.x > l + 0.0005 && p.x < r - 0.0005 {
            p.on = false;
        }
    }
    let mut boundary = |x: f64| -> Option<usize> {
        if x == 0. {
            return Some(0);
        }
        if x == 1. {
            return Some(CAP + 1);
        }
        if let Some(i) = next
            .points
            .iter()
            .position(|p| p.on && (p.x - x).abs() < 0.0005)
        {
            next.points[i].x = x;
            next.points[i].y = 0.;
            return Some(i + 1);
        }
        let i = next.points.iter().position(|p| !p.on)?;
        next.points[i] = mseg::Point { x, y: 0., on: true };
        next.bends[i + 1] = Default::default();
        Some(i + 1)
    };
    let Some(start) = boundary(l) else {
        return vec![];
    };
    let Some(end) = boundary(r) else {
        return vec![];
    };
    next.enabled = true;
    next.curve = 0.;
    if start == 0 || end == CAP + 1 {
        next.start = 0.;
    }
    next.bends[start].wave = kind;
    next.bends[start].depth = depth;
    next.bends[start].outgoing = 0.;
    next.bends[end].incoming = 0.;
    let mut e = vec![
        ("mseg_enable".into(), 1.),
        ("mseg_curve".into(), 0.),
        ("mseg_start".into(), next.start as f32),
    ];
    for (i, p) in next.points.iter().enumerate() {
        e.extend([
            (point_id(i, "x"), p.x as f32),
            (point_id(i, "y"), p.y as f32),
            (point_id(i, "on"), if p.on { 1. } else { 0. }),
        ]);
    }
    for (i, b) in next.bends.iter().enumerate() {
        e.extend([
            (node_id(i, "in"), b.incoming as f32),
            (node_id(i, "out"), b.outgoing as f32),
            (node_id(i, "wave"), b.wave as f32),
            (node_id(i, "depth"), b.depth as f32),
        ]);
    }
    *c = next;
    e
}
// draw alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
// Gövdedeki başlıca çağrılar: Config::from_params → c.shape → panel::text → panel::button. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn draw(s: &mut String, p: &ModelFParams, state: &State) {
    let c = Config::from_params(p);
    let shape = c.shape();
    let _ = write!(
        s,
        r##"<rect x="28" y="132" width="1064" height="542" rx="8" fill="#1b2123" stroke="#596265"/>"##
    );
    panel::text(s, 50., 166., 16, "#e2e5de", "MULTI-STAGE ENVELOPE", "start");
    if !c.enabled { s.push_str(r##"<g opacity="0.25">"##); }
    panel::button(s, 812., 144., 70., 30., "SAVE", false);
    panel::button(s, 894., 144., 70., 30., "LOAD", false);
    if !c.enabled { s.push_str("</g>"); }
    panel::text(s, 1068., 165., 11, "#ed6c46", "CLOSE ×", "end");
    let _ = write!(s, r##"<rect x="430" y="492" width="236" height="124" rx="6" fill="#171c1e" stroke="{}"/>"##, if c.enabled { "#343d40" } else { "#e5e7df" });
    panel::text(s, 440., 505., 8, "#a8b0ae", "AD ENVELOPE", "start");
    for ctl in CONTROLS {
        if !enabled(p, ctl.id) { s.push_str(r##"<g opacity="0.25">"##); }
        let param = p.get(ctl.id).unwrap();
        let v = param.value();
        let label = match ctl.id {
            "mseg_mode" => ["ONCE", "LOOP", "HOLD"][v as usize].into(),
            "mseg_target" => mseg::TARGETS[(v as usize).saturating_sub(1).min(2)].into(),
            "mseg_gridx" => format!("X  {}", 2usize.pow(v as u32)),
            "mseg_gridy" => format!("Y  {}", 2usize.pow(v as u32)),
            "mseg_brush" => format!("BRUSH: {}", mseg::WAVES[v as usize]),
            "mseg_snap" => format!("SNAP {}", if v > 0.5 { "ON" } else { "OFF" }),
            "mseg_enable" => if v > 0.5 { "MULTI ON" } else { "AD MODE" }.into(),
            "mseg_duration" => format!("{v:0.2} s"),
            _ => format!("{:0.0}%", v * 100.),
        };
        if ctl.knob {
            knob(
                s,
                ctl.x,
                ctl.y,
                ctl.label,
                &label,
                param.unmodulated_normalized_value(),
            );
        } else {
            if ctl.y > 500. {
                panel::text(
                    s,
                    ctl.x + ctl.w / 2.,
                    519.,
                    9,
                    "#899598",
                    ctl.label,
                    "middle",
                );
            }
            panel::button(
                s,
                ctl.x,
                ctl.y,
                ctl.w,
                ctl.h,
                &label,
                matches!(ctl.id, "mseg_enable" | "mseg_snap") && v > 0.5,
            );
        }
        if !enabled(p, ctl.id) { s.push_str("</g>"); }
    }
    if !c.enabled { s.push_str(r##"<g opacity="0.25">"##); }
    let depth = value(p, &node_id(state.selected, "depth"));
    knob(
        s,
        870.,
        558.,
        "NODE DEPTH",
        &format!("{:0.0}%", depth * 100.),
        depth as f32,
    );
    panel::button(
        s,
        536.,
        190.,
        190.,
        30.,
        &format!(
            "NODE: {}",
            mseg::WAVES[value(p, &node_id(state.selected, "wave")) as usize]
        ),
        false,
    );
    panel::text(
        s,
        750.,
        209.,
        10,
        "#a8b0ae",
        &format!(
            "{} / 16 POINTS   ·   NODE {}",
            shape.count - 2,
            state.selected
        ),
        "start",
    );
    let _ = write!(
        s,
        r##"<rect x="56" y="244" width="1008" height="240" fill="#171c1e" stroke="#41494b"/>"##
    );
    let gx = 2usize.pow(p.mseg_gridx.value() as u32);
    let gy = 2usize.pow(p.mseg_gridy.value() as u32);
    for i in 1..gx {
        let x = PLOT.0 + PLOT.2 * i as f64 / gx as f64;
        let _ = write!(s, r##"<path d="M{x} 244V484" stroke="#30393b"/>"##);
    }
    for i in 1..gy {
        let y = PLOT.1 + PLOT.3 * i as f64 / gy as f64;
        let _ = write!(s, r##"<path d="M56 {y}H1064" stroke="#30393b"/>"##);
    }
    let mut path = String::new();
    for i in 0..=1008 {
        let t = i as f64 / 1008.;
        let (x, y) = pixel(t, shape.value(t));
        let _ = write!(path, "{} {x:0.2} {y:0.2} ", if i == 0 { "M" } else { "L" });
    }
    let _ = write!(
        s,
        r##"<path d="{path} L1064 484 L56 484 Z" fill="#ed6c46" opacity="0.08"/><path d="{path}" fill="none" stroke="#ed6c46" stroke-width="2"/>"##
    );
    for n in &shape.nodes[..shape.count] {
        let (x, y) = pixel(n.x, n.y);
        let color = if n.id == state.selected {
            "#ffffff"
        } else {
            "#ff956f"
        };
        let _ = write!(
            s,
            r##"<circle cx="{x}" cy="{y}" r="5" fill="#202628" stroke="{color}" stroke-width="2"/>"##
        );
    }
    for inc in [true, false] {
        if let Some((x, y, _)) = curve_handle(&shape, state.selected, inc) {
            let (x, y) = pixel(x, y);
            if let Some(n) = shape.nodes[..shape.count]
                .iter()
                .find(|n| n.id == state.selected)
            {
                let (a, b) = pixel(n.x, n.y);
                let _ = write!(
                    s,
                    r##"<path d="M{a} {b}L{x} {y}" stroke="#a6b0ae" stroke-dasharray="3 3"/><rect x="{}" y="{}" width="8" height="8" fill="#d8dad5"/>"##,
                    x - 4.,
                    y - 4.
                );
            }
        }
    }
    if !c.enabled { s.push_str("</g>"); }
    panel::text(
        s,
        56.,
        632.,
        10,
        "#a8b0ae",
        "DRAG points / square curve handles   ·   DOUBLE-CLICK add / remove   ·   CTRL + DRAG wave brush",
        "start",
    );
    panel::text(
        s,
        56.,
        654.,
        9,
        "#899598",
        if c.enabled {
            "MULTI: graph controls modulation. Arrow keys move the selected point; Shift is fine. ESC closes."
        } else {
            "AD MODE: Attack / Decay control modulation. Enable MULTI to play the editable graph. ESC closes."
        },
        "start",
    );
}
// knob alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
// Gövdedeki başlıca çağrılar: panel::text → f64::from → to_radians → a.cos. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn knob(s: &mut String, x: f64, y: f64, label: &str, value: &str, n: f32) {
    panel::text(s, x, y - 40., 9, "#899598", label, "middle");
    let _ = write!(
        s,
        r##"<circle cx="{x}" cy="{y}" r="28" fill="#111719" stroke="#414b4e" stroke-width="2"/><circle cx="{x}" cy="{y}" r="23" fill="#282e30"/>"##
    );
    let a = (-225. + 270. * f64::from(n)).to_radians();
    let _ = write!(
        s,
        r##"<path d="M{} {}L{} {}" stroke="#ed6c46" stroke-width="3" stroke-linecap="round"/>"##,
        x + 17. * a.cos(),
        y + 17. * a.sin(),
        x + 24. * a.cos(),
        y + 24. * a.sin()
    );
    panel::text(s, x, y + 48., 11, "#ed6c46", value, "middle");
}
// wheel: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
// Gövdedeki başlıca çağrılar: node_id → hypot → dynamic.as_str → CONTROLS.iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
pub fn wheel(
    p: &ModelFParams,
    x: f64,
    y: f64,
    selected: usize,
    amount: f32,
    mods: Modifiers,
) -> Edits {
    let dynamic = node_id(selected, "depth");
    let id = if (x - 870.).hypot(y - 558.) < 40. {
        dynamic.as_str()
    } else if let Some(c) = CONTROLS.iter().find(|c| c.hit(x, y)) {
        c.id
    } else {
        return vec![];
    };
    if !enabled(p, id) { return vec![]; }
    let param = p.get(id).unwrap();
    let step = if mods.contains(Modifiers::SHIFT) {
        0.002
    } else {
        0.02
    };
    let knob = id.ends_with("depth") || CONTROLS.iter().any(|c| c.id == id && c.knob);
    let v = if knob {
        param.preview_plain((param.unmodulated_normalized_value() + amount * step).clamp(0., 1.))
    } else {
        let lo = param.preview_plain(0.);
        let hi = param.preview_plain(1.);
        (param.value() + amount.signum()).clamp(lo, hi)
    };
    vec![(id.into(), v)]
}

#[cfg(test)]
mod tests {
    use super::*;
    use nih_plug::context::process::TestProcessContext;
    use std::sync::Arc;
    // Regresyon senaryosu apply: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    fn apply(p: &ModelFParams, ctx: &mut TestProcessContext<crate::ModelF>, edits: Edits) {
        for (id, v) in edits {
            let param = p.get(&id).unwrap();
            assert!(ctx.automate(param.as_ptr(), param.preview_normalized(v)));
        }
    }
    #[test]
    // Regresyon senaryosu graph_add_drag_delete_and_curve_gestures: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Arc::new → ModelFParams::default → new → p.clone. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn graph_add_drag_delete_and_curve_gestures() {
        let p = Arc::new(ModelFParams::default());
        let mut ctx = TestProcessContext::<crate::ModelF>::new(
            p.clone() as Arc<dyn Params>,
            48000.,
            ProcessMode::Offline,
        );
        assert!(ctx.automate(p.mseg_enable.as_ptr(), 1.0));
        let mut state = State::default();
        let (x, y) = pixel(0.55, 0.45);
        let (e, _) = state.down(&p, x, y, false, Modifiers::empty());
        assert!(e.is_empty());
        let (e, _) = state.down(&p, x, y, false, Modifiers::empty());
        apply(&p, &mut ctx, e);
        assert_eq!(state.selected, 7);
        assert_eq!(Config::from_params(&p).shape().count, 9);
        state.last = None;
        let (e, _) = state.down(&p, x, y, false, Modifiers::empty());
        apply(&p, &mut ctx, e);
        let (a, b) = pixel(0.53, 0.7);
        let e = state.motion(&p, a, b, Modifiers::empty());
        apply(&p, &mut ctx, e);
        state.drag = None;
        assert!((p.mseg_p6_y.value() - 0.7).abs() < 0.001);
        let shape = Config::from_params(&p).shape();
        let (hx, hy, _) = curve_handle(&shape, 7, true).unwrap();
        let (hx, hy) = pixel(hx, hy);
        state.last = None;
        state.down(&p, hx, hy, false, Modifiers::empty());
        let e = state.motion(&p, hx, hy - 20., Modifiers::empty());
        apply(&p, &mut ctx, e);
        assert!(p.mseg_n7_in.value().abs() > 0.01);
        assert_eq!(p.mseg_n7_out.value(), 0.);
        let e = state.key(&p, &Key::Delete, Modifiers::empty());
        apply(&p, &mut ctx, e);
        assert_eq!(p.mseg_p6_on.value(), 0.);
        assert_eq!(Config::from_params(&p).shape().count, 8);
    }
    #[test]
    // Regresyon senaryosu wave_brush_populates_stable_slots_and_preserves_outside_cells: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Config::default → paint_cell → edits.is_empty → c.shape. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn wave_brush_populates_stable_slots_and_preserves_outside_cells() {
        let mut c = Config::default();
        let outside = c.points[4];
        let edits = paint_cell(&mut c, 0, 8, 0.7, 1);
        assert!(!edits.is_empty());
        assert!(c.enabled);
        assert_eq!(c.points[4], outside);
        assert_eq!(c.bends[0].wave, 1);
        let shape = c.shape();
        assert!(shape.value(0.0625) > 0.69);
        assert_eq!(shape.value(0.125), 0.);
        for cell in 0..16 {
            paint_cell(&mut c, cell, 16, 0.5, 2);
        }
        assert!(c.points.iter().filter(|p| p.on).count() <= 16);
        assert!(
            c.shape().nodes[..c.shape().count]
                .windows(2)
                .all(|n| n[0].x < n[1].x)
        );
    }
    #[test]
    // Regresyon senaryosu overlay_renders_at_both_scales_with_spaced_controls: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: ModelFParams::default → Default::default → panel::svg → svg.contains. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn overlay_renders_at_both_scales_with_spaced_controls() {
        let p = ModelFParams::default();
        let v = crate::panel::View {
            envelope: true,
            ..Default::default()
        };
        let svg = panel::svg(&p, &v);
        assert!(svg.contains("MULTI-STAGE ENVELOPE"));
        assert!(panel::render(&svg, 1120, 760).is_ok());
        assert!(panel::render(&svg, 1680, 1140).is_ok());
        for (a, c) in CONTROLS.iter().enumerate() {
            assert!(p.get(c.id).is_some());
            for other in &CONTROLS[a + 1..] {
                assert!(!other.hit(c.x, c.y), "{} / {}", c.id, other.id);
            }
        }
        // Inspect only the overlay's text: the covered synth labels remain in the
        // complete SVG but are not visible. Use the real bundled font's bounds.
        let mut overlay = String::from(
            r#"<svg xmlns="http://www.w3.org/2000/svg" width="1120" height="760"><g font-family="URW Gothic">"#,
        );
        draw(&mut overlay, &p, &v.env);
        overlay.push_str("</g></svg>");
        let mut db = resvg::usvg::fontdb::Database::new();
        db.load_font_data(
            include_bytes!("../../../third_party/urw-gothic/URWGothic-Book.otf").to_vec(),
        );
        let options = resvg::usvg::Options {
            fontdb: Arc::new(db),
            ..Default::default()
        };
        let tree = resvg::usvg::Tree::from_str(&overlay, &options).unwrap();
        // texts alt adımı: Envelope editörünün nokta/tutamak hit-test, ızgara, snap ve düzenleme işlemlerini yönetir.
        fn texts(g: &resvg::usvg::Group, out: &mut Vec<(String, resvg::usvg::Rect)>) {
            for node in g.children() {
                match node {
                    resvg::usvg::Node::Group(g) => texts(g, out),
                    resvg::usvg::Node::Text(t) => out.push((
                        t.chunks().iter().map(|c| c.text()).collect(),
                        t.abs_bounding_box(),
                    )),
                    _ => (),
                }
            }
        }
        let mut labels = Vec::new();
        texts(tree.root(), &mut labels);
        assert!(labels.len() > 25);
        for (i, (name, a)) in labels.iter().enumerate() {
            for (other, b) in &labels[i + 1..] {
                let gap = 4.;
                assert!(
                    !(a.x() < b.right() + gap
                        && a.right() + gap > b.x()
                        && a.y() < b.bottom() + gap
                        && a.bottom() + gap > b.y()),
                    "Envelope text clearance: {name} / {other}"
                );
            }
        }
    }
}
