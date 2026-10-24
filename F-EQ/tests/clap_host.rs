// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
//! CLAP lifecycle, MIDI timing, automation and session-state integration tests.
#![allow(unsafe_code, clippy::unwrap_used, dead_code)]

// (Linked for its `clap_entry`, which nothing else here names.)
extern crate simple_eq;

use std::ffi::{CStr, c_char, c_void};
use std::ptr::{null, null_mut};

use clap_sys::audio_buffer::clap_audio_buffer;
use clap_sys::entry::clap_plugin_entry;
use clap_sys::events::{
    CLAP_CORE_EVENT_SPACE_ID, CLAP_EVENT_IS_LIVE, CLAP_EVENT_MIDI, CLAP_EVENT_PARAM_VALUE,
    clap_event_header, clap_event_midi, clap_event_param_value, clap_input_events,
    clap_output_events,
};
use clap_sys::ext::params::{
    CLAP_EXT_PARAMS, clap_host_params, clap_param_clear_flags, clap_param_rescan_flags,
    clap_plugin_params,
};
use clap_sys::ext::state::{CLAP_EXT_STATE, clap_plugin_state};
use clap_sys::factory::plugin_factory::{CLAP_PLUGIN_FACTORY_ID, clap_plugin_factory};
use clap_sys::host::clap_host;
use clap_sys::id::clap_id;
use clap_sys::plugin::clap_plugin;
use clap_sys::process::clap_process;
use clap_sys::stream::{clap_istream, clap_ostream};
use clap_sys::version::CLAP_VERSION;
use std::sync::atomic::{AtomicUsize, Ordering};

unsafe extern "C" {
    /// The plug-in's CLAP entry point (`nih_export_clap!`), linked from this crate.
    static clap_entry: clap_plugin_entry;
}

const RATE: f64 = 48_000.0;
const BLOCK: u32 = 256;

/// A parameter's CLAP id: nih-plug's hash of its string id (`wrapper::util::hash_param_id`).
// id alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
fn id(param: &str) -> clap_id {
    let mut h: u32 = 0;
    for b in param.bytes() {
        h = h.wrapping_mul(31).wrapping_add(u32::from(b));
    }
    h & !(1 << 31)
}

/// The host's count of rescans asked of it, of any kind (`host_data`).
// rescan alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn rescan(host: *const clap_host, _: clap_param_rescan_flags) {
    // SAFETY: `host_data` is the instance's counter, alive as long as its host.
    unsafe { (*(*host).host_data.cast::<AtomicUsize>()).fetch_add(1, Ordering::SeqCst) };
}
// Bu bileşenin çalışma koleksiyonunu/hafızasını temizler; canlı kaynak nesnesini silmekle karıştırmayın.
unsafe extern "C" fn clear(_: *const clap_host, _: clap_id, _: clap_param_clear_flags) {}
// request alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
unsafe extern "C" fn request(_: *const clap_host) {}

static HOST_PARAMS: clap_host_params = clap_host_params {
    rescan: Some(rescan),
    clear: Some(clear),
    request_flush: Some(request),
};

// host_extension alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn host_extension(_: *const clap_host, ext: *const c_char) -> *const c_void {
    // SAFETY: the plug-in passes an extension's name.
    if unsafe { CStr::from_ptr(ext) } == CLAP_EXT_PARAMS {
        (&raw const HOST_PARAMS).cast()
    } else {
        null()
    }
}

/// An event for the plug-in.
#[derive(Clone, Copy, Debug)]
enum In {
    Note(u32, u8, bool),
    /// A control change: time, channel (0 to 15), controller, value (0 to 127).
    Cc(u32, u8, u8, u8),
    /// The host's automation: time, parameter, value (CLAP's: normalized for a knob, the
    /// position for a selector).
    Param(u32, &'static str, f64),
}

/// A parameter value the plug-in told the host: time, flags, parameter, value.
#[derive(Clone, Copy, Debug, PartialEq)]
struct Out {
    time: u32,
    flags: u32,
    param: clap_id,
    value: f64,
}

/// The events of one process call, in the host's order (each boxed: `headers` points into it).
#[allow(clippy::vec_box)]
struct Events {
    headers: Vec<*const clap_event_header>,
    _midi: Vec<Box<clap_event_midi>>,
    _params: Vec<Box<clap_event_param_value>>,
}

// in_size alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn in_size(list: *const clap_input_events) -> u32 {
    // SAFETY: `ctx` is the call's `Events`.
    unsafe {
        let events = &*(*list).ctx.cast::<Events>();
        events.headers.len() as u32
    }
}

// in_get alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn in_get(list: *const clap_input_events, i: u32) -> *const clap_event_header {
    // SAFETY: as above.
    unsafe {
        let events = &*(*list).ctx.cast::<Events>();
        events.headers.get(i as usize).copied().unwrap_or(null())
    }
}

// out_push alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Gövdedeki başlıca çağrılar: out.push. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn out_push(
    list: *const clap_output_events,
    e: *const clap_event_header,
) -> bool {
    // SAFETY: `ctx` is the call's `Vec<Out>`; the plug-in's event is valid for the call.
    unsafe {
        let out = &mut *(*list).ctx.cast::<Vec<Out>>();
        if (*e).space_id == CLAP_CORE_EVENT_SPACE_ID && (*e).type_ == CLAP_EVENT_PARAM_VALUE {
            let p = &*e.cast::<clap_event_param_value>();
            out.push(Out {
                time: p.header.time,
                flags: p.header.flags,
                param: p.param_id,
                value: p.value,
            });
        }
    }
    true
}

// write: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn write(stream: *const clap_ostream, buffer: *const c_void, size: u64) -> i64 {
    // SAFETY: `ctx` is a `Vec<u8>`; the plug-in's buffer holds `size` bytes.
    unsafe {
        let v = &mut *(*stream).ctx.cast::<Vec<u8>>();
        v.extend_from_slice(std::slice::from_raw_parts(
            buffer.cast::<u8>(),
            size as usize,
        ));
    }
    size as i64
}

// read: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn read(stream: *const clap_istream, buffer: *mut c_void, size: u64) -> i64 {
    // SAFETY: `ctx` is a `(Vec<u8>, usize)` (the bytes and where reading is); the plug-in's
    // buffer has room for `size`.
    unsafe {
        let (v, at) = &mut *(*stream).ctx.cast::<(Vec<u8>, usize)>();
        let n = (v.len() - *at).min(size as usize);
        std::ptr::copy_nonoverlapping(v[*at..].as_ptr(), buffer.cast::<u8>(), n);
        *at += n;
        n as i64
    }
}

/// An instance of the plug-in, activated and processing, as a host has it.
struct Instance {
    audio: std::sync::Mutex<Vec<f32>>,
    plugin: *const clap_plugin,
    _host: Box<clap_host>,
    /// How often the plug-in asked its host to rescan its parameters.
    rescans: Box<AtomicUsize>,
}

impl Instance {
    // Nesneyi ve sahip olduğu başlangıç durumunu oluşturur; tampon/bağımlılık ömrü bu kurulumla başlar.
    // Gövdedeki başlıca çağrılar: Box::new → AtomicUsize::new → cast_mut → cast. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn new() -> Instance {
        let rescans = Box::new(AtomicUsize::new(0));
        let host = Box::new(clap_host {
            clap_version: CLAP_VERSION,
            host_data: (&raw const *rescans).cast_mut().cast(),
            name: c"Simple EQ tests".as_ptr(),
            vendor: c"".as_ptr(),
            url: c"".as_ptr(),
            version: c"0".as_ptr(),
            get_extension: Some(host_extension),
            request_restart: Some(request),
            request_process: Some(request),
            request_callback: Some(request),
        });
        // SAFETY: the plug-in's own entry point and factory, used as CLAP has them.
        unsafe {
            let entry = &clap_entry;
            assert!((entry.init.unwrap())(c"".as_ptr()));
            let factory = (entry.get_factory.unwrap())(CLAP_PLUGIN_FACTORY_ID.as_ptr())
                .cast::<clap_plugin_factory>();
            let plugin = ((*factory).create_plugin.unwrap())(
                factory,
                &*host,
                c"com.fazilirak.simple-eq".as_ptr(),
            );
            assert!(!plugin.is_null());
            assert_eq!(
                CStr::from_ptr((*(*plugin).desc).vendor).to_str().unwrap(),
                "FAZIL IRAK"
            );
            assert!(((*plugin).init.unwrap())(plugin));
            assert!(((*plugin).activate.unwrap())(plugin, RATE, 1, BLOCK));
            assert!(((*plugin).start_processing.unwrap())(plugin));
            Instance {
                audio: std::sync::Mutex::new(Vec::new()),
                plugin,
                _host: host,
                rescans,
            }
        }
    }

    // extension alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn extension<T>(&self, name: &CStr) -> &T {
        // SAFETY: the plug-in's extension of that name is a `T`.
        unsafe {
            let e = ((*self.plugin).get_extension.unwrap())(self.plugin, name.as_ptr());
            assert!(!e.is_null(), "{name:?}");
            &*e.cast::<T>()
        }
    }

    /// One block with `events` in this order; what the plug-in told the host of parameters.
    // İlgili giriş verisini bu bileşenin mevcut durumuyla işler; alt çağrılardaki birim ve kanal sınırlarını koruyun.
    // Gövdedeki başlıca çağrılar: Vec::new → In::Note → Box::new → ev.headers.push. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn process(&self, events: &[In]) -> Vec<Out> {
        let mut ev = Events {
            headers: Vec::new(),
            _midi: Vec::new(),
            _params: Vec::new(),
        };
        for &e in events {
            match e {
                In::Note(time, note, on) => {
                    let m = Box::new(clap_event_midi {
                        header: clap_event_header {
                            size: size_of::<clap_event_midi>() as u32,
                            time,
                            space_id: CLAP_CORE_EVENT_SPACE_ID,
                            type_: CLAP_EVENT_MIDI,
                            flags: CLAP_EVENT_IS_LIVE,
                        },
                        port_index: 0,
                        data: [if on { 0x90 } else { 0x80 }, note, if on { 100 } else { 0 }],
                    });
                    ev.headers.push(&raw const m.header);
                    ev._midi.push(m);
                }
                In::Cc(time, channel, cc, value) => {
                    let m = Box::new(clap_event_midi {
                        header: clap_event_header {
                            size: size_of::<clap_event_midi>() as u32,
                            time,
                            space_id: CLAP_CORE_EVENT_SPACE_ID,
                            type_: CLAP_EVENT_MIDI,
                            flags: CLAP_EVENT_IS_LIVE,
                        },
                        port_index: 0,
                        data: [0xb0 | channel, cc, value],
                    });
                    ev.headers.push(&raw const m.header);
                    ev._midi.push(m);
                }
                In::Param(time, param, value) => {
                    let p = Box::new(clap_event_param_value {
                        header: clap_event_header {
                            size: size_of::<clap_event_param_value>() as u32,
                            time,
                            space_id: CLAP_CORE_EVENT_SPACE_ID,
                            type_: CLAP_EVENT_PARAM_VALUE,
                            flags: 0,
                        },
                        param_id: id(param),
                        cookie: null_mut(),
                        note_id: -1,
                        port_index: -1,
                        channel: -1,
                        key: -1,
                        value,
                    });
                    ev.headers.push(&raw const p.header);
                    ev._params.push(p);
                }
            }
        }
        let n = BLOCK as usize;
        let (mut out_l, mut out_r, mut in_l, mut in_r) = (
            vec![0.0f32; n],
            vec![0.0f32; n],
            vec![0.0f32; n],
            vec![0.0f32; n],
        );
        for (i, v) in in_l.iter_mut().enumerate() {
            *v = 0.2 * (i as f32 * 0.13).sin();
        }
        let expected = in_l.clone();
        let mut outs = [out_l.as_mut_ptr(), out_r.as_mut_ptr()];
        let mut ins = [in_l.as_mut_ptr(), in_r.as_mut_ptr()];
        let mut output = clap_audio_buffer {
            data32: outs.as_mut_ptr(),
            data64: null_mut(),
            channel_count: 2,
            latency: 0,
            constant_mask: 0,
        };
        let _input = clap_audio_buffer {
            data32: ins.as_mut_ptr(),
            data64: null_mut(),
            channel_count: 2,
            latency: 0,
            constant_mask: 0,
        };
        let in_events = clap_input_events {
            ctx: (&raw mut ev).cast(),
            size: Some(in_size),
            get: Some(in_get),
        };
        let mut told: Vec<Out> = Vec::new();
        let out_events = clap_output_events {
            ctx: (&raw mut told).cast(),
            try_push: Some(out_push),
        };
        let process = clap_process {
            steady_time: -1,
            frames_count: BLOCK,
            transport: null(),
            audio_inputs: &_input,
            audio_outputs: &mut output,
            audio_inputs_count: 1,
            audio_outputs_count: 1,
            in_events: &in_events,
            out_events: &out_events,
        };
        // SAFETY: a process call as CLAP has it, everything it points to alive through it.
        unsafe {
            ((*self.plugin).process.unwrap())(self.plugin, &process);
        }
        assert!(out_l.iter().chain(&out_r).all(|v| v.is_finite()));
        if self.value("analog") < 0.5 {
            assert!(
                out_r.iter().all(|v| v.abs() < 1e-9),
                "Left channel must not leak into right"
            );
        } else {
            assert!(
                out_r.iter().all(|v| v.abs() < 0.001),
                "Silent channel contains more than the expected subtle analog noise"
            );
        }
        let _ = expected;
        *self.audio.lock().unwrap() = out_l;
        told
    }

    /// A parameter's value as the host reads it (CLAP's: normalized for a knob, the position
    /// for a selector).
    // value alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn value(&self, param: &str) -> f64 {
        let params: &clap_plugin_params = self.extension(CLAP_EXT_PARAMS);
        let mut v = f64::NAN;
        // SAFETY: as CLAP has it.
        assert!(unsafe { (params.get_value.unwrap())(self.plugin, id(param), &mut v) });
        v
    }

    /// The plug-in's state as the host saves it.
    // save: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn save(&self) -> Vec<u8> {
        let state: &clap_plugin_state = self.extension(CLAP_EXT_STATE);
        let mut bytes: Vec<u8> = Vec::new();
        let stream = clap_ostream {
            ctx: (&raw mut bytes).cast(),
            write: Some(write),
        };
        // SAFETY: as CLAP has it.
        assert!(unsafe { (state.save.unwrap())(self.plugin, &stream) });
        bytes
    }

    /// A state loaded as a host loads one: whether the plug-in took it.
    // load: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn load(&self, bytes: &[u8]) -> bool {
        let state: &clap_plugin_state = self.extension(CLAP_EXT_STATE);
        let mut source = (bytes.to_vec(), 0usize);
        let stream = clap_istream {
            ctx: (&raw mut source).cast(),
            read: Some(read),
        };
        // SAFETY: as CLAP has it.
        unsafe { (state.load.unwrap())(self.plugin, &stream) }
    }
}

impl Drop for Instance {
    // Kapanışta sahip olunan kaynak ve bağlantıları bırakır; callback’in geçersiz nesneye erişmemesi gerekir.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn drop(&mut self) {
        // SAFETY: as CLAP has it, once.
        unsafe {
            ((*self.plugin).stop_processing.unwrap())(self.plugin);
            ((*self.plugin).deactivate.unwrap())(self.plugin);
            ((*self.plugin).destroy.unwrap())(self.plugin);
        }
    }
}

#[test]
// clap_audio_automation_bypass_and_session_roundtrip alt adımı: Bu modülün clap_host veri/işleme sözleşmesini ve çağıran bileşenlere sunduğu yardımcı adımları uygular.
// Gövdedeki başlıca çağrılar: Instance::new → a.process → a.audio.lock → unwrap. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
fn clap_audio_automation_bypass_and_session_roundtrip() {
    let a = Instance::new();
    a.process(&[]);
    for (i, y) in a.audio.lock().unwrap().iter().enumerate() {
        assert!((y - 0.2 * (i as f32 * 0.13).sin()).abs() < 1e-6);
    }
    // Volume range -24..12 dB: normalized 5/6 = +6 dB.
    a.process(&[In::Param(96, "volume", 5.0 / 6.0)]);
    for (i, y) in a.audio.lock().unwrap()[..96].iter().enumerate() {
        assert!(
            (y - 0.2 * (i as f32 * 0.13).sin()).abs() < 1e-6,
            "Automation arrived early at {i}: {y}"
        );
    }
    for _ in 0..8 {
        a.process(&[]);
    }
    for (i, y) in a.audio.lock().unwrap().iter().enumerate() {
        assert!((y - 0.2 * (i as f32 * 0.13).sin() * 10f32.powf(6. / 20.)).abs() < 1e-5);
    }
    a.process(&[
        In::Param(0, "tilt", 0.75),
        In::Param(0, "q", 0.85),
        In::Param(0, "bass_q", 0.2),
        In::Param(0, "low_cut", 0.45),
        In::Param(0, "high_cut", 0.7),
        In::Param(0, "analog", 1.0),
        In::Param(0, "saturation", 0.6),
        In::Param(0, "low_slope", 4.0),
        In::Param(0, "high_slope", 3.0),
    ]);
    let saved = a.save();
    let b = Instance::new();
    assert!(b.load(&saved));
    for param in [
        "volume",
        "tilt",
        "pivot",
        "bass",
        "q",
        "bass_q",
        "low_cut",
        "high_cut",
        "bypass",
        "analog",
        "saturation",
        "low_slope",
        "high_slope",
    ] {
        assert!(
            (a.value(param) - b.value(param)).abs() < 1e-6,
            "State mismatch: {param}"
        );
    }
    a.process(&[In::Param(0, "bypass", 1.0)]);
    for _ in 0..8 {
        a.process(&[]);
    }
    for (i, y) in a.audio.lock().unwrap().iter().enumerate() {
        assert!(
            (y - 0.2 * (i as f32 * 0.13).sin()).abs() < 1e-6,
            "Bypass must restore dry signal including volume"
        );
    }
}
