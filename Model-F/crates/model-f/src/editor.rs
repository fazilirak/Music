// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 24.10.2026
// Dosyanın görevi: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
use crate::{
    panel::{self, CONTROLS, H, View, W},
    params::ModelFParams,
    presets::{self, Preset},
    window,
};
use baseview::{
    Event, EventStatus, MouseButton, MouseEvent, ScrollDelta, Size, Window, WindowEvent,
    WindowHandler, WindowOpenOptions, WindowScalePolicy,
};
use keyboard_types::{Code, Key, KeyState, Modifiers};
use nih_plug::prelude::*;
use std::{
    any::Any,
    sync::{
        Arc,
        atomic::{AtomicBool, AtomicU32, Ordering},
    },
    time::{Duration, Instant},
};

pub struct ModelFEditor {
    params: Arc<ModelFParams>,
    meter: Arc<AtomicU32>,
    overload: Arc<AtomicU32>,
    keys: Arc<AtomicU32>,
    panic_requested: Arc<AtomicBool>,
    scale: AtomicU32,
    open: Arc<AtomicBool>,
}
pub struct Closing {
    open: Arc<AtomicBool>,
    keys: Arc<AtomicU32>,
}
impl Drop for Closing {
    // Kapanışta sahip olunan kaynak ve bağlantıları bırakır; callback’in geçersiz nesneye erişmemesi gerekir.
    fn drop(&mut self) {
        self.open.store(false, Ordering::Release);
        self.keys.store(0, Ordering::Release);
    }
}
impl ModelFEditor {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    pub fn new(
        params: Arc<ModelFParams>,
        meter: Arc<AtomicU32>,
        overload: Arc<AtomicU32>,
        keys: Arc<AtomicU32>,
        panic_requested: Arc<AtomicBool>,
    ) -> Self {
        Self {
            params,
            meter,
            overload,
            keys,
            panic_requested,
            scale: AtomicU32::new(1.0f32.to_bits()),
            open: Arc::new(AtomicBool::new(false)),
        }
    }
}
impl Editor for ModelFEditor {
    // spawn: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: window::Parent → self.open.clone → self.keys.clone → parent.can_open. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn spawn(
        &self,
        parent: ParentWindowHandle,
        context: Arc<dyn GuiContext>,
    ) -> Box<dyn Any + Send> {
        let parent = window::Parent(parent);
        let closing = Closing {
            open: self.open.clone(),
            keys: self.keys.clone(),
        };
        if !parent.can_open() {
            return Box::new(window::Handle::new(None, closing));
        }
        let (p, m, o, k, panic) = (
            self.params.clone(),
            self.meter.clone(),
            self.overload.clone(),
            self.keys.clone(),
            self.panic_requested.clone(),
        );
        let scale = f64::from(f32::from_bits(self.scale.load(Ordering::Relaxed)));
        let handle = Window::open_parented(
            &parent,
            WindowOpenOptions {
                title: "Model F".into(),
                size: Size::new(f64::from(W), f64::from(H)),
                scale: WindowScalePolicy::ScaleFactor(scale),
            },
            move |w| PanelWindow::new(w, p, m, o, k, panic, context, scale),
        );
        self.open.store(true, Ordering::Release);
        Box::new(window::Handle::new(Some(handle), closing))
    }
    // size alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn size(&self) -> (u32, u32) {
        (W, H)
    }
    // set_scale_factor: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_scale_factor(&self, factor: f32) -> bool {
        if self.open.load(Ordering::Acquire) || !factor.is_finite() || factor <= 0.0 {
            return false;
        }
        self.scale.store(factor.to_bits(), Ordering::Relaxed);
        true
    }
    // param_value_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_value_changed(&self, _: &str, _: f32) {}
    // param_modulation_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_modulation_changed(&self, _: &str, _: f32) {}
    // param_values_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_values_changed(&self) {}
}
struct PanelWindow {
    params: Arc<ModelFParams>,
    meter: Arc<AtomicU32>,
    overload: Arc<AtomicU32>,
    keys: Arc<AtomicU32>,
    panic_requested: Arc<AtomicBool>,
    context: Arc<dyn GuiContext>,
    surface: window::Surface,
    physical: (u32, u32),
    logical: (f64, f64),
    view: View,
    bank: Vec<Preset>,
    selected: Option<Preset>,
    pointer: (f64, f64),
    drag: Option<(usize, f64, f32)>,
    mouse_note: Option<u32>,
    keyboard_notes: u32,
    keyboard_active: bool,
    last_svg: String,
    last_draw: Instant,
    pixels: Option<resvg::tiny_skia::Pixmap>,
    options: resvg::usvg::Options<'static>,
    focus: Option<window::Keys>,
}
impl PanelWindow {
    // env_finish alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn env_finish(&mut self) {
        self.view.env.drag = None;
        let setter = ParamSetter::new(self.context.as_ref());
        for id in self.view.env.gestures.drain(..) {
            if let Some(p) = self.params.get(&id) {
                setter.end_set_parameter(p);
            }
        }
    }
    // env_apply alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: ParamSetter::new → self.context.as_ref → self.params.get → setter.begin_set_parameter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn env_apply(&mut self, edits: crate::mseg_ui::Edits, once: bool) {
        let setter = ParamSetter::new(self.context.as_ref());
        for (id, v) in edits {
            if let Some(p) = self.params.get(&id) {
                if once {
                    setter.begin_set_parameter(p);
                } else if !self.view.env.gestures.contains(&id) {
                    setter.begin_set_parameter(p);
                    self.view.env.gestures.push(id);
                }
                setter.set_parameter(p, v);
                if once {
                    setter.end_set_parameter(p);
                }
            }
        }
    }
    // envelope_file alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: self.env_finish → cfg → rfd::FileDialog::new → set_title. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn envelope_file(&mut self, save: bool) {
        self.env_finish();
        #[cfg(any(target_os = "windows", target_os = "macos"))]
        {
            let dialog = rfd::FileDialog::new()
                .set_title(if save {
                    "Save Model F envelope"
                } else {
                    "Load Model F envelope"
                })
                .add_filter("Model F envelope", &["modelfenv"]);
            let path = if save {
                dialog.set_file_name("Envelope.modelfenv").save_file()
            } else {
                dialog.pick_file()
            };
            if let Some(path) = path {
                let result = if save {
                    crate::mseg::EnvelopePreset::capture(&self.params).save(&path)
                } else {
                    crate::mseg::EnvelopePreset::read(&path).and_then(|p| {
                        p.apply(&self.params, &ParamSetter::new(self.context.as_ref()))
                    })
                };
                self.view.status = match result {
                    Ok(()) => if save {
                        "Envelope saved."
                    } else {
                        "Envelope loaded."
                    }
                    .into(),
                    Err(e) => e,
                };
            }
        }
    }
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: f64::from → round → resvg::usvg::fontdb::Database::new → db.load_font_data. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn new(
        window: &mut Window<'_>,
        params: Arc<ModelFParams>,
        meter: Arc<AtomicU32>,
        overload: Arc<AtomicU32>,
        keys: Arc<AtomicU32>,
        panic_requested: Arc<AtomicBool>,
        context: Arc<dyn GuiContext>,
        scale: f64,
    ) -> Self {
        let physical = (
            (f64::from(W) * scale).round() as u32,
            (f64::from(H) * scale).round() as u32,
        );
        let mut db = resvg::usvg::fontdb::Database::new();
        db.load_font_data(
            include_bytes!("../../../third_party/urw-gothic/URWGothic-Book.otf").to_vec(),
        );
        let mut this = Self {
            surface: window::Surface::new(window, physical),
            physical,
            logical: (f64::from(W), f64::from(H)),
            params,
            meter,
            overload,
            keys,
            panic_requested,
            context,
            view: View {
                category: "INIT".into(),
                filter: "Bass".into(),
                status: "Ready. Connect MIDI or click the keys to play.".into(),
                ..View::default()
            },
            bank: Vec::new(),
            selected: None,
            pointer: (0.0, 0.0),
            drag: None,
            mouse_note: None,
            keyboard_notes: 0,
            keyboard_active: false,
            last_svg: String::new(),
            last_draw: Instant::now() - Duration::from_secs(1),
            pixels: None,
            options: resvg::usvg::Options {
                fontdb: Arc::new(db),
                ..Default::default()
            },
            focus: None,
        };
        this.reload();
        let name = this
            .params
            .preset_name
            .read()
            .map(|n| n.clone())
            .unwrap_or_default();
        this.selected = this.bank.iter().find(|p| p.name == name).cloned();
        this.refresh_rows();
        this
    }
    // reload alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn reload(&mut self) {
        self.bank = presets::factory();
        let (users, errors) = presets::user();
        self.bank.extend(users);
        if !errors.is_empty() {
            self.view.status = format!("{} user preset file(s) could not be read", errors.len());
        }
    }
    // refresh_rows alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: self.view.search.to_lowercase → presets::factory → len → iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn refresh_rows(&mut self) {
        let search = self.view.search.to_lowercase();
        let factory_count = presets::factory().len();
        self.view.rows = self
            .bank
            .iter()
            .enumerate()
            .filter(|(i, p)| {
                (if self.view.filter == "User" {
                    *i >= factory_count
                } else {
                    p.category == self.view.filter
                }) && (p.name.to_lowercase().contains(&search)
                    || p.description.to_lowercase().contains(&search))
            })
            .map(|(_, p)| p.clone())
            .collect();
        self.view.offset = self
            .view
            .offset
            .min(self.view.rows.len().saturating_sub(20));
    }
    // load: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    fn load(&mut self, preset: Preset) {
        self.end_drag();
        match preset.apply(&self.params, &ParamSetter::new(self.context.as_ref())) {
            Ok(()) => {
                self.view.status = preset.description.clone();
                self.view.category = preset.category.clone();
                self.selected = Some(preset);
            }
            Err(e) => self.view.status = e,
        }
    }
    // step_preset alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: self.refresh_rows → rows.is_empty → as_ref → and_then. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn step_preset(&mut self, delta: isize) {
        self.refresh_rows();
        let rows = &self.view.rows;
        if rows.is_empty() {
            return;
        }
        let current = self
            .selected
            .as_ref()
            .and_then(|p| rows.iter().position(|r| r.name == p.name));
        let n = current.map_or(if delta > 0 { 0 } else { rows.len() - 1 }, |n| {
            (n as isize + delta).rem_euclid(rows.len() as isize) as usize
        });
        let p = rows[n].clone();
        self.load(p);
    }
    // end_drag alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn end_drag(&mut self) {
        if let Some((i, _, _)) = self.drag.take() {
            if let Some(p) = self.params.get(CONTROLS[i].id) {
                ParamSetter::new(self.context.as_ref()).end_set_parameter(p);
            }
        }
    }
    // set_normalized: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_normalized(&self, i: usize, value: f32, once: bool) {
        let p = self.params.get(CONTROLS[i].id).expect("known control");
        let setter = ParamSetter::new(self.context.as_ref());
        // Fazıl Irak, 24.10.2026: eski ortak ayarı ilk düzenlemede ayır; diğer düğmeler yerinden oynamaz.
        let group: &[&str] = match CONTROLS[i].id {
            "audio_attack" | "audio_decay" | "audio_sustain" => &["audio_attack", "audio_decay", "audio_sustain"],
            "osc2_waveform" | "osc2_octave" => &["osc2_waveform", "osc2_octave"],
            _ => &[],
        };
        for &id in group {
            let param = self.params.get(id).unwrap();
            if param.unmodulated_plain_value() < 0.0 {
                let value = self.params.effective(id);
                setter.begin_set_parameter(param); setter.set_parameter(param,value); setter.end_set_parameter(param);
            }
        }
        if once {
            setter.begin_set_parameter(p);
        }
        // -1 yalnızca eski dosya geçişi içindir; Audio ADS düğmesinin gerçek hareketi 0–10.
        let value=value.clamp(0.0,1.0);
        let normalized=if matches!(CONTROLS[i].id,"audio_attack" | "audio_decay" | "audio_sustain") {p.preview_normalized(value*10.0)} else {value};
        setter.set_parameter_normalized(p, normalized);
        if once {
            setter.end_set_parameter(p);
        }
    }
    // selector alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn selector(&self, i: usize, delta: f32) {
        let p = self.params.get(CONTROLS[i].id).expect("known control");
        let lo = p.preview_plain(0.0);
        let hi = p.preview_plain(1.0);
        let value = self.params.effective(CONTROLS[i].id);
        let lo = if matches!(CONTROLS[i].id, "osc2_waveform" | "osc2_octave") { 0.0 } else { lo };
        let next = if value + delta > hi {
            lo
        } else if value + delta < lo {
            hi
        } else {
            value + delta
        };
        self.set_normalized(i, p.preview_normalized(next), true);
    }
    // publish_keys alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn publish_keys(&self) {
        self.keys.store(
            self.keyboard_notes | self.mouse_note.map_or(0, |n| 1 << n),
            Ordering::Release,
        );
    }
    // release_keys alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn release_keys(&mut self) {
        self.keyboard_notes = 0;
        self.mouse_note = None;
        self.keys.store(0, Ordering::Release);
    }
    // save_user: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    // Gövdedeki başlıca çağrılar: Preset::capture → self.view.save_name.trim → into → presets::save_user. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn save_user(&mut self) {
        let preset = Preset::capture(
            &self.params,
            self.view.save_name.trim().into(),
            presets::CATEGORIES[self.view.save_category].into(),
        );
        match presets::save_user(&preset) {
            Ok(_) => {
                if let Ok(mut n) = self.params.preset_name.write() {
                    *n = preset.name.clone();
                }
                self.selected = Some(preset);
                self.view.save = false;
                self.view.status = "Sound saved to your Model F preset library.".into();
                self.reload();
                self.refresh_rows();
            }
            Err(e) => self.view.status = e,
        }
    }
    // import alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: cfg → rfd::FileDialog::new → set_title → add_filter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn import(&mut self) {
        #[cfg(any(target_os = "windows", target_os = "macos"))]
        if let Some(path) = rfd::FileDialog::new()
            .set_title("Load Model F sound")
            .add_filter("Model F sound", &["modelf"])
            .pick_file()
        {
            match presets::read(&path) {
                Ok(p) => {
                    self.view.category = p.category.clone();
                    self.load(p);
                }
                Err(e) => self.view.status = e,
            }
        }
    }
    // export alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: cfg → read → map → n.clone. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn export(&mut self) {
        #[cfg(any(target_os = "windows", target_os = "macos"))]
        {
            let name = self
                .params
                .preset_name
                .read()
                .map(|n| n.clone())
                .unwrap_or_else(|_| "Model F sound".into());
            let preset = Preset::capture(
                &self.params,
                name,
                if presets::CATEGORIES.contains(&self.view.category.as_str()) {
                    self.view.category.clone()
                } else {
                    "Bass".into()
                },
            );
            if let Some(path) = rfd::FileDialog::new()
                .set_title("Export Model F sound")
                .add_filter("Model F sound", &["modelf"])
                .set_file_name(format!("{}.modelf", preset.name))
                .save_file()
            {
                self.view.status = match presets::save(&path, &preset) {
                    Ok(()) => "Sound exported.".into(),
                    Err(e) => e,
                };
            }
        }
    }
    // click: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: let → panel::in_rect → self.end_drag → self.env_finish. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn click(&mut self, window: &mut Window<'_>, right: bool, modifiers: Modifiers) {
        let (x, y) = self.pointer;
        // The visible emergency button remains available above modal input handling.
        if !right && panel::in_rect(panel::PANIC_RECT, x, y) {
            self.end_drag();
            self.env_finish();
            self.release_keys();
            self.keyboard_active = false;
            self.panic_requested.store(true, Ordering::Release);
            self.view.status = "PANIC — all notes cleared.".into();
            return;
        }
        if self.view.about {
            self.view.about_click(x, y);
            return;
        }
        // Klavye bir kez etkinleştirilince knob'a tıklamak odağı host'a geri vermez.
        if !right && panel::in_rect((28., 687., 198., 43.), x, y) {
            self.keyboard_active = true;
            window.set_wants_keys(true);
            if self.focus.is_none() { self.focus = window::take_keys(window); }
            self.view.status = "Computer keyboard on: A W S E D F T G Y H U J K. ESC releases.".into();
            return;
        }
        if self.view.save {
            if (393.0..427.0).contains(&y) {
                if (340.0..540.0).contains(&x) {
                    self.view.save_category = (self.view.save_category + 1) % 6;
                } else if (562.0..662.0).contains(&x) {
                    self.view.save = false;
                } else if (680.0..780.0).contains(&x) {
                    self.save_user();
                }
            }
            return;
        }
        if !right && panel::in_rect(panel::LOGO_RECT, x, y) {
            self.env_finish();
            self.end_drag();
            self.release_keys();
            self.view.open_about();
            if self.focus.is_none() {
                self.focus = window::take_keys(window);
            }
            return;
        }
        if !right && panel::in_rect(crate::mseg_ui::OPEN, x, y) {
            self.end_drag();
            self.env_finish();
            self.release_keys();
            self.view.envelope = !self.view.envelope;
            self.view.browser = false;
            if self.focus.is_none() {
                self.focus = window::take_keys(window);
            }
            return;
        }
        if self.view.envelope && panel::in_rect(crate::mseg_ui::RECT, x, y) {
            self.env_finish();
            let (edits, command) = self.view.env.down(&self.params, x, y, right, modifiers);
            let once = self.view.env.drag.is_none();
            self.env_apply(edits, once);
            match command {
                crate::mseg_ui::Command::Close => {
                    self.env_finish();
                    self.view.envelope = false;
                }
                crate::mseg_ui::Command::Save => self.envelope_file(true),
                crate::mseg_ui::Command::Load => self.envelope_file(false),
                _ => (),
            }
            return;
        }
        if self.view.browser {
            if (140.0..174.0).contains(&y) && x > 980.0 {
                self.view.browser = false;
                return;
            }
            if (185.0..215.0).contains(&y) && (48.0..748.0).contains(&x) {
                let i = ((x - 48.0) / 100.0) as usize;
                self.view.filter =
                    ["Bass", "Lead", "Key", "Pad", "FX", "Arp", "User"][i.min(6)].into();
                self.view.offset = 0;
                self.refresh_rows();
                return;
            }
            if (234.0..594.0).contains(&y) && (48.0..1072.0).contains(&x) {
                let col = usize::from(x >= 568.0);
                let row = ((y - 234.0) / 36.0) as usize;
                let n = self.view.offset + col * 10 + row;
                if let Some(p) = self.view.rows.get(n).cloned() {
                    self.load(p);
                }
            }
            return;
        }
        if (40.0..78.0).contains(&y) {
            if (340.0..770.0).contains(&x) {
                self.env_finish();
                self.view.envelope = false;
                self.release_keys();
                self.view.browser = true;
                self.reload();
                self.refresh_rows();
                if self.focus.is_none() {
                    self.focus = window::take_keys(window);
                }
                return;
            }
            if (782.0..812.0).contains(&x) {
                self.step_preset(-1);
                return;
            }
            if (820.0..850.0).contains(&x) {
                self.step_preset(1);
                return;
            }
            if (868.0..924.0).contains(&x) {
                self.release_keys();
                self.view.save = true;
                self.view.save_name.clear();
                self.view.status = "Choose a name; click category to change it.".into();
                if self.focus.is_none() {
                    self.focus = window::take_keys(window);
                }
                return;
            }
            if (934.0..990.0).contains(&x) {
                self.import();
                return;
            }
        }
        if (84.0..107.0).contains(&y) && (868.0..990.0).contains(&x) {
            self.export();
            return;
        }
        if (687.0..730.0).contains(&y) {
            if (420.0..996.0).contains(&x) {
                self.keyboard_active = true;
                window.set_wants_keys(true);
                self.mouse_note = Some(((x - 420.0) / 24.0) as u32);
                self.publish_keys();
                if self.focus.is_none() {
                    self.focus = window::take_keys(window);
                }
                return;
            }
        }
        if let Some(i) = CONTROLS.iter().position(|c| c.hit(x, y)) {
            let c = &CONTROLS[i];
            if !panel::control_enabled(&self.params, c.id) { return; }
            if c.id == "waveform" && self.params.mode.value().round() == 1.0 {
                self.view.status = "SuperSaw always uses seven Model F saw oscillators.".into();
                return;
            }
            if modifiers.contains(Modifiers::CONTROL) || modifiers.contains(Modifiers::ALT) {
                if let Some(p) = self.params.get(c.id) {
                    self.set_normalized(i, p.default_normalized_value(), true);
                }
                return;
            }
            if c.knob && !right {
                let p = self.params.get(c.id).expect("known control");
                ParamSetter::new(self.context.as_ref()).begin_set_parameter(p);
                self.drag = Some((i, y, self.params.display_normalized(c.id)));
            } else if !c.knob {
                self.selector(i, if right { -1.0 } else { 1.0 });
            }
        }
    }
    // key: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: self.env_finish → self.view.env.key → edits.is_empty → self.env_apply. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn key(&mut self, e: keyboard_types::KeyboardEvent) -> bool {
        if self.view.about {
            if e.state == KeyState::Down && matches!(e.key, Key::Escape | Key::Enter) {
                self.view.about = false;
            }
            return true;
        }
        if self.view.envelope && e.state == KeyState::Down {
            if e.key == Key::Escape {
                self.env_finish();
                self.view.envelope = false;
                return true;
            }
            let edits = self.view.env.key(&self.params, &e.key, e.modifiers);
            if !edits.is_empty() {
                self.env_apply(edits, true);
                return true;
            }
        }
        if self.view.save || self.view.browser {
            if e.state != KeyState::Down {
                return true;
            }
            match e.key {
                Key::Escape => {
                    self.view.save = false;
                    self.view.browser = false;
                }
                Key::Enter => {
                    if self.view.save {
                        self.save_user();
                    } else {
                        self.view.browser = false;
                    }
                }
                Key::Backspace => {
                    if self.view.save {
                        self.view.save_name.pop();
                    } else {
                        self.view.search.pop();
                        self.view.offset = 0;
                        self.refresh_rows();
                    }
                }
                Key::ArrowDown => self.step_preset(1),
                Key::ArrowUp => self.step_preset(-1),
                Key::Character(ref c) => {
                    if !e.modifiers.intersects(Modifiers::CONTROL | Modifiers::META) {
                        if self.view.save {
                            if self.view.save_name.chars().count() < 64 {
                                self.view.save_name.push_str(c);
                            }
                        } else {
                            self.view.search.push_str(c);
                            self.view.offset = 0;
                            self.refresh_rows();
                        }
                    }
                }
                _ => (),
            }
            return true;
        }
        if e.key == Key::Escape && e.state == KeyState::Down {
            self.release_keys(); self.keyboard_active = false; return true;
        }
        let n = if self.keyboard_active || self.view.envelope { piano_note(&e) } else { None };
        if let Some(n) = n {
            if e.state == KeyState::Down {
                self.keyboard_notes |= 1 << n;
                self.view.status = format!("Computer keyboard: {}{}  /  MIDI {}", ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"][(n%12) as usize], 3+n/12, 48+n);
            } else {
                self.keyboard_notes &= !(1 << n);
            }
            self.publish_keys();
            true
        } else {
            false
        }
    }
    // draw alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: read → map → n.clone → unwrap_or_default. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn draw(&mut self) {
        self.view.name = self
            .params
            .preset_name
            .read()
            .map(|n| n.clone())
            .unwrap_or_default();
        if let Some(p) = self.bank.iter().find(|p| p.name == self.view.name) {
            if self.selected.as_ref().is_none_or(|old| old.name != p.name) {
                self.selected = Some(p.clone());
            }
        }
        if let Some(p) = &self.selected {
            self.view.category = p.category.clone();
            self.view.dirty = p.differs(&self.params);
        }
        self.view.keys = self.keys.load(Ordering::Acquire) & 0x00ffffff;
        self.view.meter =
            (f32::from_bits(self.meter.load(Ordering::Relaxed)) * 16.0).round() / 16.0;
        self.view.overload =
            (f32::from_bits(self.overload.load(Ordering::Relaxed)).clamp(0.0, 1.0) * 32.0).round()
                / 32.0;
        let svg = panel::svg(&self.params, &self.view);
        if svg != self.last_svg {
            if let Ok(tree) = resvg::usvg::Tree::from_str(&svg, &self.options) {
                if let Some(mut px) =
                    resvg::tiny_skia::Pixmap::new(self.physical.0, self.physical.1)
                {
                    resvg::render(
                        &tree,
                        resvg::tiny_skia::Transform::from_scale(
                            self.physical.0 as f32 / W as f32,
                            self.physical.1 as f32 / H as f32,
                        ),
                        &mut px.as_mut(),
                    );
                    self.pixels = Some(px);
                    self.last_svg = svg;
                }
            }
        }
        if let Some(px) = &self.pixels {
            self.surface.present(self.physical, &[px]);
        }
    }
}
impl WindowHandler for PanelWindow {
    // on_frame: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: self.drag.is_some → self.view.env.drag.is_some → window::lost_capture → self.end_drag. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn on_frame(&mut self, window: &mut Window<'_>) {
        if (self.drag.is_some() || self.view.env.drag.is_some()) && window::lost_capture(window) {
            self.end_drag();
            self.env_finish();
        }
        window.set_wants_keys(
            self.view.about
                || self.view.browser
                || self.view.save
                || self.view.envelope
                || self.keyboard_active,
        );
        if !self.view.about
            && !self.view.browser
            && !self.view.save
            && !self.view.envelope
            && !self.keyboard_active
        {
            if let Some(keys) = self.focus.take() {
                window::give_keys_back(window, keys);
            }
        }
        if self.last_draw.elapsed() >= Duration::from_millis(33) {
            self.draw();
            self.last_draw = Instant::now();
        }
    }
    // on_event: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: Event::Window → WindowEvent::Resized → info.physical_size → size.width.max. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn on_event(&mut self, window: &mut Window<'_>, event: Event) -> EventStatus {
        match event {
            Event::Window(WindowEvent::Resized(info)) => {
                let size = info.physical_size();
                self.physical = (size.width.max(1), size.height.max(1));
                self.logical = (
                    info.logical_size().width.max(1.0),
                    info.logical_size().height.max(1.0),
                );
                self.surface.resize(self.physical);
                self.last_svg.clear();
            }
            Event::Window(WindowEvent::Unfocused | WindowEvent::WillClose) => {
                self.end_drag();
                self.env_finish();
                self.release_keys();
                self.keyboard_active = false;
            }
            Event::Window(WindowEvent::Damaged) => self.draw(),
            Event::Mouse(MouseEvent::CursorMoved {
                position,
                modifiers,
            }) => {
                self.pointer = (
                    position.x * f64::from(W) / self.logical.0,
                    position.y * f64::from(H) / self.logical.1,
                );
                if self.view.envelope && self.view.env.drag.is_some() {
                    let edits = self.view.env.motion(
                        &self.params,
                        self.pointer.0,
                        self.pointer.1,
                        modifiers,
                    );
                    self.env_apply(edits, false);
                    return EventStatus::Captured;
                }
                self.view.hover = CONTROLS
                    .iter()
                    .position(|c| c.hit(self.pointer.0, self.pointer.1));
                if let Some((i, y, base)) = self.drag {
                    let fine = if modifiers.contains(Modifiers::SHIFT) {
                        0.0005
                    } else {
                        0.005
                    };
                    self.set_normalized(i, base + ((y - self.pointer.1) * fine) as f32, false);
                    self.view.status = format!(
                        "{}: {}   /   SHIFT fine  •  CTRL-click reset",
                        CONTROLS[i].label,
                        panel::value(&self.params, CONTROLS[i].id)
                    );
                }
            }
            Event::Mouse(MouseEvent::ButtonPressed { button, modifiers }) => {
                if button == MouseButton::Left || button == MouseButton::Right {
                    self.click(window, button == MouseButton::Right, modifiers);
                }
            }
            Event::Mouse(MouseEvent::ButtonReleased { .. }) => {
                self.end_drag();
                self.env_finish();
                self.mouse_note = None;
                self.publish_keys();
                window::release_capture(window);
            }
            Event::Mouse(MouseEvent::WheelScrolled { delta, modifiers }) => {
                if self.view.about {
                    return EventStatus::Captured;
                }
                let amount = match delta {
                    ScrollDelta::Lines { y, .. } => y,
                    ScrollDelta::Pixels { y, .. } => y / 40.0,
                };
                if self.view.envelope {
                    let edits = crate::mseg_ui::wheel(
                        &self.params,
                        self.pointer.0,
                        self.pointer.1,
                        self.view.env.selected,
                        amount,
                        modifiers,
                    );
                    self.env_apply(edits, true);
                    return EventStatus::Captured;
                }
                if self.view.browser {
                    if amount < 0.0 {
                        self.view.offset =
                            (self.view.offset + 20).min(self.view.rows.len().saturating_sub(20));
                    } else {
                        self.view.offset = self.view.offset.saturating_sub(20);
                    }
                } else if !self.view.save {
                    if let Some(i) = self.view.hover {
                        if !panel::control_enabled(&self.params, CONTROLS[i].id) { return EventStatus::Captured; }
                        if CONTROLS[i].knob {
                            let step = if modifiers.contains(Modifiers::SHIFT) {
                                0.002
                            } else {
                                0.02
                            };
                            self.set_normalized(
                                i,
                                self.params.display_normalized(CONTROLS[i].id) + amount * step,
                                true,
                            );
                        } else {
                            self.selector(i, amount.signum());
                        }
                    }
                }
            }
            Event::Keyboard(e) => {
                if !self.key(e) {
                    return EventStatus::Ignored;
                }
            }
            _ => return EventStatus::Ignored,
        }
        EventStatus::Captured
    }
}

// Fiziksel scan-code, Türkçe Q/F karakter dönüşümü ve Shift/CapsLock'tan bağımsız nota tutar.
// KeyUp sırasında modifier değişmiş olsa da nota kapanır. Metin kutuları bu yoldan önce işlenir.
fn piano_note(e: &keyboard_types::KeyboardEvent) -> Option<u32> {
    if e.state == KeyState::Down && e.modifiers.intersects(Modifiers::CONTROL | Modifiers::META | Modifiers::ALT) { return None; }
    match e.code {
        Code::KeyA=>Some(0), Code::KeyW=>Some(1), Code::KeyS=>Some(2), Code::KeyE=>Some(3),
        Code::KeyD=>Some(4), Code::KeyF=>Some(5), Code::KeyT=>Some(6), Code::KeyG=>Some(7),
        Code::KeyY=>Some(8), Code::KeyH=>Some(9), Code::KeyU=>Some(10), Code::KeyJ=>Some(11), Code::KeyK=>Some(12),
        _ => if let Key::Character(ref key) = e.key {
            ["a","w","s","e","d","f","t","g","y","h","u","j","k"].iter().position(|k| *k == key.to_lowercase()).map(|n| n as u32)
        } else { None }
    }
}

#[cfg(test)]
mod keyboard_tests {
    use super::*;
    #[test]
    fn physical_keys_survive_layout_shift_and_modifier_release() {
        let mut e=keyboard_types::KeyboardEvent::default();
        e.code=Code::KeyA; e.key=Key::Character("ş".into()); e.state=KeyState::Down;
        assert_eq!(piano_note(&e),Some(0));
        e.modifiers=Modifiers::SHIFT; assert_eq!(piano_note(&e),Some(0));
        e.modifiers=Modifiers::CONTROL; assert_eq!(piano_note(&e),None);
        e.state=KeyState::Up; assert_eq!(piano_note(&e),Some(0));
        e.code=Code::Unidentified; e.modifiers=Modifiers::empty(); e.key=Key::Character("K".into());
        assert_eq!(piano_note(&e),Some(12));
    }
}
