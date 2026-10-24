// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
use crate::{panel, params::EqParams, window};
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
pub struct EqEditor {
    params: Arc<EqParams>,
    meter: Arc<AtomicU32>,
    rate: Arc<AtomicU32>,
    spectrum: Arc<crate::spectrum::Analyzer>,
    scale: AtomicU32,
    open: Arc<AtomicBool>,
    about_keys: Arc<AboutKeys>,
}
#[derive(Default)]
struct AboutKeys {
    open: AtomicBool,
    close: AtomicBool,
}
pub struct Closing {
    open: Arc<AtomicBool>,
    about_keys: Arc<AboutKeys>,
}
impl Drop for Closing {
    // Kapanışta sahip olunan kaynak ve bağlantıları bırakır; callback’in geçersiz nesneye erişmemesi gerekir.
    fn drop(&mut self) {
        self.open.store(false, Ordering::Release);
        self.about_keys.open.store(false, Ordering::Release);
        self.about_keys.close.store(false, Ordering::Release);
    }
}
impl EqEditor {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    pub fn new(
        params: Arc<EqParams>,
        meter: Arc<AtomicU32>,
        rate: Arc<AtomicU32>,
        spectrum: Arc<crate::spectrum::Analyzer>,
    ) -> Self {
        Self {
            params,
            meter,
            rate,
            spectrum,
            scale: AtomicU32::new(1f32.to_bits()),
            open: Arc::new(AtomicBool::new(false)),
            about_keys: Arc::new(AboutKeys::default()),
        }
    }
}
impl Editor for EqEditor {
    // spawn: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: window::Parent → self.open.clone → self.about_keys.clone → parent.can_open. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn spawn(
        &self,
        parent: ParentWindowHandle,
        context: Arc<dyn GuiContext>,
    ) -> Box<dyn Any + Send> {
        let parent = window::Parent(parent);
        let closing = Closing {
            open: self.open.clone(),
            about_keys: self.about_keys.clone(),
        };
        if !parent.can_open() {
            return Box::new(window::Handle::new(None, closing));
        }
        let p = self.params.clone();
        let m = self.meter.clone();
        let r = self.rate.clone();
        let spectrum = self.spectrum.clone();
        let about_keys = self.about_keys.clone();
        let scale = f32::from_bits(self.scale.load(Ordering::Relaxed)) as f64;
        let handle = Window::open_parented(
            &parent,
            WindowOpenOptions {
                title: "F EQ".into(),
                size: Size::new(panel::W as f64, panel::H as f64),
                scale: WindowScalePolicy::ScaleFactor(scale),
                #[cfg(feature = "standalone")]
                gl_config: None,
            },
            move |w| PanelWindow::new(w, p, m, r, spectrum, about_keys, context, scale),
        );
        self.open.store(true, Ordering::Release);
        Box::new(window::Handle::new(Some(handle), closing))
    }
    // size alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn size(&self) -> (u32, u32) {
        (panel::W, panel::H)
    }
    // set_scale_factor: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_scale_factor(&self, f: f32) -> bool {
        if self.open.load(Ordering::Acquire) || !f.is_finite() || f <= 0. {
            return false;
        }
        self.scale.store(f.to_bits(), Ordering::Relaxed);
        true
    }
    // param_value_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_value_changed(&self, _: &str, _: f32) {}
    // param_modulation_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_modulation_changed(&self, _: &str, _: f32) {}
    // param_values_changed alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    fn param_values_changed(&self) {}
    // host_key_down alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: self.about_keys.open.load → self.about_keys.close.store. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn host_key_down(&self, key: u16, key_code: i16, modifiers: i16) -> bool {
        // Steinberg keycodes.h: RETURN=4, ESCAPE=6, keypad ENTER=19.
        if modifiers == 0
            && self.about_keys.open.load(Ordering::Acquire)
            && (matches!(key, 0x1b | 0x0d | 0x0a) || matches!(key_code, 4 | 6 | 19))
        {
            // The host can call outside baseview's window callback. Queue the
            // close for the GUI frame; never reenter drawing/focus from here.
            self.about_keys.close.store(true, Ordering::Release);
            true
        } else {
            false
        }
    }
}
struct PanelWindow {
    params: Arc<EqParams>,
    meter: Arc<AtomicU32>,
    rate: Arc<AtomicU32>,
    spectrum: Arc<crate::spectrum::Analyzer>,
    spectrum_bins: [f32; crate::spectrum::BINS],
    spectrum_cursor: usize,
    spectrum_last_data: Instant,
    context: Arc<dyn GuiContext>,
    surface: window::Surface,
    physical: (u32, u32),
    logical: (f64, f64),
    pointer: (f64, f64),
    drag: Option<(usize, f64, f32)>,
    last: Instant,
    last_svg: String,
    pixels: Option<resvg::tiny_skia::Pixmap>,
    options: resvg::usvg::Options<'static>,
    display_meter: f32,
    about: bool,
    focus: Option<window::Keys>,
    about_keys: Arc<AboutKeys>,
}
impl PanelWindow {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: round → Instant::now → window::Surface::new → Duration::from_secs. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn new(
        w: &mut Window<'_>,
        params: Arc<EqParams>,
        meter: Arc<AtomicU32>,
        rate: Arc<AtomicU32>,
        spectrum: Arc<crate::spectrum::Analyzer>,
        about_keys: Arc<AboutKeys>,
        context: Arc<dyn GuiContext>,
        scale: f64,
    ) -> Self {
        let physical = (
            (panel::W as f64 * scale).round() as u32,
            (panel::H as f64 * scale).round() as u32,
        );
        Self {
            params,
            meter,
            rate,
            spectrum,
            spectrum_bins: [-100.; crate::spectrum::BINS],
            spectrum_cursor: 0,
            spectrum_last_data: Instant::now(),
            context,
            surface: window::Surface::new(w, physical),
            physical,
            logical: (panel::W as f64, panel::H as f64),
            pointer: (0., 0.),
            drag: None,
            last: Instant::now() - Duration::from_secs(1),
            last_svg: String::new(),
            pixels: None,
            options: panel::options(),
            display_meter: 0.,
            about: false,
            focus: None,
            about_keys,
        }
    }
    // finish: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    fn finish(&mut self) {
        if let Some((i, _, _)) = self.drag.take() {
            ParamSetter::new(self.context.as_ref()).end_set_parameter(self.params.knobs()[i]);
        }
    }
    // set_about: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set_about(&mut self, w: &mut Window<'_>, open: bool) {
        self.finish();
        self.about = open;
        self.about_keys.close.store(false, Ordering::Release);
        self.about_keys.open.store(open, Ordering::Release);
        w.set_wants_keys(open);
        if open {
            self.focus = window::take_keys(w);
        } else if let Some(keys) = self.focus.take() {
            window::give_keys_back(w, keys);
        }
        self.draw();
    }
    // set: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir. Burada verilen ayar türetilmiş duruma/çıktıya aktarılır; çağrı sırası aşağıdaki gövdededir.
    fn set(&self, i: usize, v: f32, once: bool) {
        let p = self.params.knobs()[i];
        let setter = ParamSetter::new(self.context.as_ref());
        if once {
            setter.begin_set_parameter(p);
        }
        setter.set_parameter_normalized(p, panel::host_normalized(&self.params, i, v));
        if once {
            setter.end_set_parameter(p);
        }
    }
    // draw alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: f32::from_bits → self.meter.load → meter.max → self.rate.load. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn draw(&mut self) {
        let meter = f32::from_bits(self.meter.load(Ordering::Relaxed));
        self.display_meter = meter.max(self.display_meter * 0.92);
        let rate = f32::from_bits(self.rate.load(Ordering::Relaxed)) as f64;
        let cursor = self.spectrum.cursor();
        if cursor != self.spectrum_cursor {
            let bins = self.spectrum.read(rate);
            for (i, v) in bins.iter().enumerate() {
                let speed = if *v > self.spectrum_bins[i] {
                    0.65
                } else {
                    0.14
                };
                self.spectrum_bins[i] += (*v - self.spectrum_bins[i]) * speed;
            }
            self.spectrum_cursor = cursor;
            self.spectrum_last_data = Instant::now();
        } else if self.spectrum_last_data.elapsed() > Duration::from_millis(120) {
            for v in &mut self.spectrum_bins {
                *v = (*v - 2.).max(-100.);
            }
        }
        let mut s =
            panel::svg_with_spectrum(&self.params, self.display_meter, rate, &self.spectrum_bins);
        if self.about {
            s = panel::with_about(&s);
        }
        if s != self.last_svg {
            self.pixels = panel::render(&s, self.physical.0, self.physical.1, &self.options);
            self.last_svg = s;
        }
        if let Some(px) = &self.pixels {
            self.surface.present(self.physical, &[px]);
        }
    }
}
impl WindowHandler for PanelWindow {
    // on_frame: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    fn on_frame(&mut self, w: &mut Window<'_>) {
        if self.about && self.about_keys.close.swap(false, Ordering::AcqRel) {
            self.set_about(w, false);
        }
        w.set_wants_keys(self.about);
        if self.drag.is_some() && window::lost_capture(w) {
            self.finish();
        }
        if self.last.elapsed() > Duration::from_millis(33) {
            self.draw();
            self.last = Instant::now();
        }
    }
    // on_event: kullanıcı/pencere olayını bu bileşenin parametre ve sahiplik durumuna taşır; kapanmış nesne ve boş seçim yollarını koruyun.
    // Gövdedeki başlıca çağrılar: Event::Window → WindowEvent::Resized → info.physical_size → s.width.max. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn on_event(&mut self, w: &mut Window<'_>, e: Event) -> EventStatus {
        match e {
            Event::Window(WindowEvent::Resized(info)) => {
                let s = info.physical_size();
                self.physical = (s.width.max(1), s.height.max(1));
                let l = info.logical_size();
                self.logical = (l.width.max(1.), l.height.max(1.));
                self.surface.resize(self.physical);
                self.last_svg.clear();
            }
            Event::Window(WindowEvent::Unfocused) => self.finish(),
            Event::Window(WindowEvent::WillClose) => {
                self.finish();
                self.about = false;
                self.about_keys.open.store(false, Ordering::Release);
                self.about_keys.close.store(false, Ordering::Release);
                w.set_wants_keys(false);
                if let Some(keys) = self.focus.take() {
                    window::give_keys_back(w, keys);
                }
            }
            Event::Window(WindowEvent::Damaged) => self.draw(),
            Event::Mouse(MouseEvent::CursorMoved {
                position,
                modifiers,
            }) => {
                self.pointer = (
                    position.x * panel::W as f64 / self.logical.0,
                    position.y * panel::H as f64 / self.logical.1,
                );
                if let Some((i, y, base)) = self.drag {
                    self.set(
                        i,
                        base + ((y - self.pointer.1)
                            * if modifiers.contains(Modifiers::SHIFT) {
                                0.0005
                            } else {
                                0.005
                            }) as f32,
                        false,
                    );
                }
            }
            Event::Mouse(MouseEvent::ButtonPressed {
                button: MouseButton::Left,
                modifiers,
            }) => {
                self.finish();
                let (x, y) = self.pointer;
                if self.about {
                    if panel::about_close_hit(x, y) || !panel::about_hit(x, y) {
                        self.set_about(w, false);
                    }
                    return EventStatus::Captured;
                }
                if panel::logo_hit(x, y) {
                    self.set_about(w, true);
                    return EventStatus::Captured;
                }
                if panel::power_hit(x, y) {
                    let setter = ParamSetter::new(self.context.as_ref());
                    setter.begin_set_parameter(&self.params.bypass);
                    setter.set_parameter(&self.params.bypass, !self.params.bypass.value());
                    setter.end_set_parameter(&self.params.bypass);
                } else if panel::analog_hit(x, y) {
                    let setter = ParamSetter::new(self.context.as_ref());
                    setter.begin_set_parameter(&self.params.analog);
                    setter.set_parameter(&self.params.analog, !self.params.analog.value());
                    setter.end_set_parameter(&self.params.analog);
                } else if let Some((low, index)) = panel::slope_hit(x, y) {
                    let p = if low {
                        &self.params.low_slope
                    } else {
                        &self.params.high_slope
                    };
                    let setter = ParamSetter::new(self.context.as_ref());
                    setter.begin_set_parameter(p);
                    setter.set_parameter(p, index as i32);
                    setter.end_set_parameter(p);
                } else if let Some(i) = panel::hit(x, y) {
                    let p = self.params.knobs()[i];
                    if modifiers.intersects(Modifiers::CONTROL | Modifiers::ALT) {
                        self.set(i, panel::default_position(&self.params, i), true);
                    } else {
                        ParamSetter::new(self.context.as_ref()).begin_set_parameter(p);
                        self.drag = Some((i, y, panel::knob_position(&self.params, i)));
                    }
                }
            }
            Event::Mouse(MouseEvent::ButtonReleased { .. }) => {
                self.finish();
                window::release_capture(w);
            }
            Event::Mouse(MouseEvent::WheelScrolled { delta, modifiers }) => {
                if self.about {
                    return EventStatus::Captured;
                }
                if let Some(i) = panel::hit(self.pointer.0, self.pointer.1) {
                    let amount = match delta {
                        ScrollDelta::Lines { y, .. } => y,
                        ScrollDelta::Pixels { y, .. } => y / 40.,
                    };
                    let step = if modifiers.contains(Modifiers::SHIFT) {
                        0.002
                    } else {
                        0.02
                    };
                    self.set(
                        i,
                        panel::knob_position(&self.params, i) + amount * step,
                        true,
                    );
                }
            }
            Event::Keyboard(e) if self.about => {
                if dismiss_key(e.state, &e.key, e.code) {
                    self.set_about(w, false);
                }
            }
            _ => return EventStatus::Ignored,
        }
        EventStatus::Captured
    }
}
// dismiss_key alt adımı: Yazılım panelini native pencereye bağlar; parametre, fare/klavye ve pencere yaşam döngüsünü yönetir.
fn dismiss_key(state: KeyState, key: &Key, code: Code) -> bool {
    state == KeyState::Down
        && (matches!(key, Key::Escape | Key::Enter)
            || matches!(code, Code::Escape | Code::Enter | Code::NumpadEnter)
            || matches!(key,Key::Character(s) if matches!(s.as_str(),"\u{1b}"|"\r"|"\n")))
}
#[cfg(test)]
mod keyboard_tests {
    use super::*;
    #[test]
    // Regresyon senaryosu vst3_host_keys_only_close_an_open_about: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: EqEditor::new → Arc::new → EqParams::default → AtomicU32::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn vst3_host_keys_only_close_an_open_about() {
        let editor = EqEditor::new(
            Arc::new(EqParams::default()),
            Arc::new(AtomicU32::new(0)),
            Arc::new(AtomicU32::new(48000f32.to_bits())),
            Arc::new(crate::spectrum::Analyzer::default()),
        );
        assert!(!editor.host_key_down(0, 6, 0));
        editor.about_keys.open.store(true, Ordering::Release);
        for (key, code) in [(27, 0), (13, 0), (10, 0), (0, 6), (0, 4), (0, 19)] {
            assert!(editor.host_key_down(key, code, 0));
            assert!(editor.about_keys.close.swap(false, Ordering::AcqRel));
        }
        assert!(!editor.host_key_down('A' as u16, 0, 0));
        assert!(!editor.host_key_down(13, 4, 4));
        assert!(!editor.about_keys.close.load(Ordering::Acquire));
    }
    #[test]
    // Regresyon senaryosu native_escape_enter_and_keypad_enter_close_but_releases_do_not: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Gövdedeki başlıca çağrılar: Key::Character → into → dismiss_key. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn native_escape_enter_and_keypad_enter_close_but_releases_do_not() {
        for (key, code) in [
            (Key::Escape, Code::Escape),
            (Key::Enter, Code::Enter),
            (Key::Enter, Code::NumpadEnter),
            (Key::Unidentified, Code::NumpadEnter),
            (Key::Character("\r".into()), Code::Unidentified),
        ] {
            assert!(dismiss_key(KeyState::Down, &key, code));
            assert!(!dismiss_key(KeyState::Up, &key, code));
        }
        assert!(!dismiss_key(
            KeyState::Down,
            &Key::Character("a".into()),
            Code::KeyA
        ));
    }
}
