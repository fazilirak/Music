// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
#![allow(unsafe_code)]
use std::num::{NonZeroIsize, NonZeroU32};
use std::panic::{AssertUnwindSafe, catch_unwind};
use std::ptr::NonNull;

use baseview::WindowHandle;
use nih_plug::editor::ParentWindowHandle;
use raw_window_handle::{
    HasRawDisplayHandle, HasRawWindowHandle, RawDisplayHandle, RawWindowHandle,
};
use raw_window_handle_06 as rwh6;

/// The host's window the editor opens in.
pub struct Parent(pub ParentWindowHandle);

impl Parent {
    /// Whether a window can open in it: the host gave one, and on Linux there is an X11
    /// display (baseview panics in the host's thread without one).
    // can_open alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    pub fn can_open(&self) -> bool {
        let given = match self.0 {
            ParentWindowHandle::X11Window(w) => w != 0,
            ParentWindowHandle::AppKitNsView(v) => !v.is_null(),
            ParentWindowHandle::Win32Hwnd(h) => !h.is_null(),
        };
        given && display()
    }
}

#[cfg(target_os = "linux")]
// display alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
fn display() -> bool {
    use x11::xlib;
    // SAFETY: a connection of our own, closed at once.
    unsafe {
        let d = xlib::XOpenDisplay(std::ptr::null());
        if d.is_null() {
            return false;
        }
        xlib::XCloseDisplay(d);
    }
    true
}

#[cfg(not(target_os = "linux"))]
// display alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
fn display() -> bool {
    true
}

// SAFETY: the handle is the host's window, alive while the editor is open, and given as
// the platform's own kind.
unsafe impl HasRawWindowHandle for Parent {
    // raw_window_handle alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: ParentWindowHandle::X11Window → raw_window_handle::XcbWindowHandle::empty → RawWindowHandle::Xcb → ParentWindowHandle::AppKitNsView. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn raw_window_handle(&self) -> RawWindowHandle {
        match self.0 {
            ParentWindowHandle::X11Window(window) => {
                let mut h = raw_window_handle::XcbWindowHandle::empty();
                h.window = window;
                RawWindowHandle::Xcb(h)
            }
            ParentWindowHandle::AppKitNsView(ns_view) => {
                let mut h = raw_window_handle::AppKitWindowHandle::empty();
                h.ns_view = ns_view;
                RawWindowHandle::AppKit(h)
            }
            ParentWindowHandle::Win32Hwnd(hwnd) => {
                let mut h = raw_window_handle::Win32WindowHandle::empty();
                h.hwnd = hwnd;
                RawWindowHandle::Win32(h)
            }
        }
    }
}

/// The open window (if one could open), closed when dropped, the editor then reset.
pub struct Handle {
    window: Option<WindowHandle>,
    _closing: crate::editor::Closing,
}

impl Handle {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    pub fn new(window: Option<WindowHandle>, closing: crate::editor::Closing) -> Self {
        Handle {
            window,
            _closing: closing,
        }
    }
}

// SAFETY: the host drops the editor's handle on its GUI thread, where it opened it; the
// raw pointers inside are not used from any other.
unsafe impl Send for Handle {}

impl Drop for Handle {
    // Kapanışta sahip olunan kaynak ve bağlantıları bırakır; callback’in geçersiz nesneye erişmemesi gerekir.
    fn drop(&mut self) {
        if let Some(w) = &mut self.window {
            w.close();
        }
    }
}

/// The keyboard as the drawer took it from another window (Windows: that window).
#[derive(Debug)]
#[cfg_attr(not(target_os = "windows"), allow(dead_code))]
pub struct Keys {
    #[cfg(target_os = "windows")]
    had: winapi::shared::windef::HWND,
}

#[cfg(target_os = "windows")]
// hwnd alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
fn hwnd(window: &baseview::Window<'_>) -> Option<winapi::shared::windef::HWND> {
    match window.raw_window_handle() {
        RawWindowHandle::Win32(h) if !h.hwnd.is_null() => Some(h.hwnd.cast()),
        _ => None,
    }
}

/// The keyboard to the editor's window (baseview's focus): on Windows, the window that
/// had it if another did. Not on X11, where baseview's focus is `unimplemented!()` (it
/// panicked the editor's thread, the window gone): there the keys go to the window under
/// the pointer within the host's focused window, so to the drawer while the pointer is
/// over the editor.
// take_keys alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Gövdedeki başlıca çağrılar: cfg → hwnd → GetFocus → window.focus. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
pub fn take_keys(window: &mut baseview::Window<'_>) -> Option<Keys> {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::winuser::GetFocus;
        let hwnd = hwnd(window)?;
        // SAFETY: the calling thread's focus, the editor's window being on it.
        let had = unsafe { GetFocus() };
        window.focus();
        (had != hwnd).then_some(Keys { had })
    }
    #[cfg(target_os = "macos")]
    {
        window.focus();
        None
    }
    #[cfg(not(any(target_os = "windows", target_os = "macos")))]
    {
        let _ = window;
        None
    }
}

/// The keyboard given back as the drawer shuts, to the window it was taken from (else
/// the host's window holding the editor's), if the editor's window still has it.
/// baseview on Windows keeps every key its window is given, the host's shortcuts too.
// give_keys_back alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Gövdedeki başlıca çağrılar: cfg → hwnd → GetFocus → keys.had.is_null. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
pub fn give_keys_back(window: &baseview::Window<'_>, keys: Keys) {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::winuser::{GetFocus, GetParent, IsWindow, SetFocus};
        let Some(hwnd) = hwnd(window) else { return };
        // SAFETY: the calling thread's focus and windows; `keys.had` is checked to be a
        // window still before it is given the focus.
        unsafe {
            if GetFocus() != hwnd {
                return;
            }
            let to = if !keys.had.is_null() && IsWindow(keys.had) != 0 {
                keys.had
            } else {
                GetParent(hwnd)
            };
            SetFocus(to);
        }
    }
    #[cfg(not(target_os = "windows"))]
    {
        let _ = (window, keys);
    }
}

/// Whether the editor's window has lost the pointer's capture (Windows: baseview
/// captures it at a press and lets it go at the release, and ignores its loss, at Alt-Tab
/// mid-drag, when no release comes). Elsewhere a release always comes.
// lost_capture alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
pub fn lost_capture(window: &baseview::Window<'_>) -> bool {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::winuser::GetCapture;
        // SAFETY: the calling thread's capture.
        hwnd(window).is_some_and(|hwnd| unsafe { GetCapture() } != hwnd)
    }
    #[cfg(not(target_os = "windows"))]
    {
        let _ = window;
        false
    }
}

/// The pointer's capture let go once no button is down (Windows). baseview counts the
/// presses and releases to let it go; a release lost with the capture leaves the count
/// one up, and its window then kept the capture after every release, the host's own
/// clicks coming to the editor.
// release_capture alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Gövdedeki başlıca çağrılar: cfg → hwnd → iter → any. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
pub fn release_capture(window: &baseview::Window<'_>) {
    #[cfg(target_os = "windows")]
    {
        use winapi::um::winuser::{
            GetCapture, GetKeyState, ReleaseCapture, VK_LBUTTON, VK_MBUTTON, VK_RBUTTON,
            VK_XBUTTON1, VK_XBUTTON2,
        };
        let Some(hwnd) = hwnd(window) else { return };
        // SAFETY: the calling thread's capture and the buttons as its messages have them.
        unsafe {
            let down = [VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2]
                .iter()
                .any(|&b| GetKeyState(b) < 0);
            if !down && GetCapture() == hwnd {
                ReleaseCapture();
            }
        }
    }
    #[cfg(not(target_os = "windows"))]
    {
        let _ = window;
    }
}

/// baseview's window as softbuffer takes it.
#[derive(Clone)]
pub struct Target {
    display: rwh6::RawDisplayHandle,
    window: rwh6::RawWindowHandle,
}

impl rwh6::HasDisplayHandle for Target {
    // display_handle alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn display_handle(&self) -> Result<rwh6::DisplayHandle<'_>, rwh6::HandleError> {
        // SAFETY: the display outlives the window drawn in, which outlives the surface.
        Ok(unsafe { rwh6::DisplayHandle::borrow_raw(self.display) })
    }
}

impl rwh6::HasWindowHandle for Target {
    // window_handle alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn window_handle(&self) -> Result<rwh6::WindowHandle<'_>, rwh6::HandleError> {
        // SAFETY: the window outlives its surface (both are the editor window's).
        Ok(unsafe { rwh6::WindowHandle::borrow_raw(self.window) })
    }
}

// target alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
// Gövdedeki başlıca çağrılar: window.raw_display_handle → RawDisplayHandle::AppKit → rwh6::RawDisplayHandle::AppKit → rwh6::AppKitDisplayHandle::new. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn target(window: &baseview::Window<'_>) -> Option<Target> {
    let display = match window.raw_display_handle() {
        RawDisplayHandle::AppKit(_) => {
            rwh6::RawDisplayHandle::AppKit(rwh6::AppKitDisplayHandle::new())
        }
        RawDisplayHandle::Xlib(h) => rwh6::RawDisplayHandle::Xlib(rwh6::XlibDisplayHandle::new(
            NonNull::new(h.display),
            h.screen,
        )),
        RawDisplayHandle::Xcb(h) => rwh6::RawDisplayHandle::Xcb(rwh6::XcbDisplayHandle::new(
            NonNull::new(h.connection),
            h.screen,
        )),
        RawDisplayHandle::Windows(_) => {
            rwh6::RawDisplayHandle::Windows(rwh6::WindowsDisplayHandle::new())
        }
        _ => return None,
    };
    let window = match window.raw_window_handle() {
        RawWindowHandle::AppKit(h) => {
            rwh6::RawWindowHandle::AppKit(rwh6::AppKitWindowHandle::new(NonNull::new(h.ns_view)?))
        }
        RawWindowHandle::Xlib(h) => {
            rwh6::RawWindowHandle::Xlib(rwh6::XlibWindowHandle::new(h.window))
        }
        RawWindowHandle::Xcb(h) => {
            rwh6::RawWindowHandle::Xcb(rwh6::XcbWindowHandle::new(NonZeroU32::new(h.window)?))
        }
        RawWindowHandle::Win32(h) => {
            let mut w = rwh6::Win32WindowHandle::new(NonZeroIsize::new(h.hwnd as isize)?);
            w.hinstance = NonZeroIsize::new(h.hinstance as isize);
            rwh6::RawWindowHandle::Win32(w)
        }
        _ => return None,
    };
    Some(Target { display, window })
}

/// softbuffer's call, or nothing if it panicked: its Win32 backend asserts that GDI
/// gave it a device context and a bitmap, which without a desktop it may not, and a
/// panic there crossed into the host (pluginval crashed, decisions.md R18).
// guarded alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
fn guarded<T>(f: impl FnOnce() -> T) -> Option<T> {
    catch_unwind(AssertUnwindSafe(f)).ok()
}

/// Where the frames are shown; nothing if the platform's window could not be drawn in.
pub struct Surface {
    inner: Option<(
        softbuffer::Context<Target>,
        softbuffer::Surface<Target, Target>,
    )>,
}

impl Surface {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    pub fn new(window: &baseview::Window<'_>, size: (u32, u32)) -> Self {
        let inner = target(window).and_then(|t| {
            guarded(|| {
                let context = softbuffer::Context::new(t.clone()).ok()?;
                let surface = softbuffer::Surface::new(&context, t).ok()?;
                Some((context, surface))
            })
            .flatten()
        });
        let mut s = Surface { inner };
        s.resize(size);
        s
    }

    // resize alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    pub fn resize(&mut self, (w, h): (u32, u32)) {
        if let (Some((_, surface)), Some(w), Some(h)) =
            (&mut self.inner, NonZeroU32::new(w), NonZeroU32::new(h))
            && guarded(|| surface.resize(w, h)).is_none()
        {
            self.inner = None;
        }
    }

    /// Shows frames (premultiplied RGBA, opaque), one under the other from the top left
    /// corner, in a window of `size` pixels.
    // present alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    pub fn present(&mut self, size: (u32, u32), frames: &[&resvg::tiny_skia::Pixmap]) {
        let Some((_, surface)) = &mut self.inner else {
            return;
        };
        if guarded(|| Self::show(surface, size, frames)).is_none() {
            self.inner = None;
        }
    }

    // show alt adımı: Native pencere/ebeveyn handle, keyboard focus, mouse capture ve piksel yüzeyi yaşam döngüsünü yönetir.
    // Gövdedeki başlıca çağrılar: surface.buffer_mut → let → buffer.len → frames.iter. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    fn show(
        surface: &mut softbuffer::Surface<Target, Target>,
        (w, h): (u32, u32),
        frames: &[&resvg::tiny_skia::Pixmap],
    ) {
        let Ok(mut buffer) = surface.buffer_mut() else {
            return;
        };
        let (w, h) = (w as usize, h as usize);
        if buffer.len() < w * h {
            return;
        }
        let mut frames = frames.iter();
        let mut frame = frames.next();
        let mut top = 0usize;
        for y in 0..h {
            while let Some(f) = frame
                && y >= top + f.height() as usize
            {
                top += f.height() as usize;
                frame = frames.next();
            }
            let row = &mut buffer[y * w..(y + 1) * w];
            for (x, out) in row.iter_mut().enumerate() {
                *out = match frame {
                    Some(f) if x < f.width() as usize => {
                        let (fw, fy) = (f.width() as usize, y - top);
                        let p = &f.data()[(fy * fw + x) * 4..(fy * fw + x) * 4 + 4];
                        (u32::from(p[0]) << 16) | (u32::from(p[1]) << 8) | u32::from(p[2])
                    }
                    _ => 0x003b_2213,
                };
            }
        }
        let _ = buffer.present();
    }
}
