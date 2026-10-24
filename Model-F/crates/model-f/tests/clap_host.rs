// Türkçe bakım açıklamaları: Fazıl Irak
// Son düzenleme: 22.10.2026
// Dosyanın görevi: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
//! CLAP lifecycle, MIDI timing, automation and session-state integration tests.
#![allow(unsafe_code, clippy::unwrap_used)]

// (Linked for its `clap_entry`, which nothing else here names.)
extern crate model_f;

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
// id alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
fn id(param: &str) -> clap_id {
    let mut h: u32 = 0;
    for b in param.bytes() {
        h = h.wrapping_mul(31).wrapping_add(u32::from(b));
    }
    h & !(1 << 31)
}

/// The host's count of rescans asked of it, of any kind (`host_data`).
// rescan alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn rescan(host: *const clap_host, _: clap_param_rescan_flags) {
    // SAFETY: `host_data` is the instance's counter, alive as long as its host.
    unsafe { (*(*host).host_data.cast::<AtomicUsize>()).fetch_add(1, Ordering::SeqCst) };
}
// Bu bileşenin çalışma koleksiyonunu/hafızasını temizler; canlı kaynak nesnesini silmekle karıştırmayın.
unsafe extern "C" fn clear(_: *const clap_host, _: clap_id, _: clap_param_clear_flags) {}
// request alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
unsafe extern "C" fn request(_: *const clap_host) {}

static HOST_PARAMS: clap_host_params = clap_host_params {
    rescan: Some(rescan),
    clear: Some(clear),
    request_flush: Some(request),
};

// host_extension alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
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

// in_size alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn in_size(list: *const clap_input_events) -> u32 {
    // SAFETY: `ctx` is the call's `Events`.
    unsafe {
        let events = &*(*list).ctx.cast::<Events>();
        events.headers.len() as u32
    }
}

// in_get alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
// Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
unsafe extern "C" fn in_get(list: *const clap_input_events, i: u32) -> *const clap_event_header {
    // SAFETY: as above.
    unsafe {
        let events = &*(*list).ctx.cast::<Events>();
        events.headers.get(i as usize).copied().unwrap_or(null())
    }
}

// out_push alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
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

// write: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
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

// read: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
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
            name: c"Model F tests".as_ptr(),
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
                c"audio.modelf.synth".as_ptr(),
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

    // Regresyon senaryosu extension: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
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
            audio_inputs: null(),
            audio_outputs: &mut output,
            audio_inputs_count: 0,
            audio_outputs_count: 1,
            in_events: &in_events,
            out_events: &out_events,
        };
        // SAFETY: a process call as CLAP has it, everything it points to alive through it.
        unsafe {
            ((*self.plugin).process.unwrap())(self.plugin, &process);
        }
        assert!(out_l.iter().chain(&out_r).all(|v| v.is_finite()));
        *self.audio.lock().unwrap() = out_l;
        told
    }

    /// A parameter's value as the host reads it (CLAP's: normalized for a knob, the position
    /// for a selector).
    // Regresyon senaryosu value: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
    // Ham işaretçi/FFI sınırı var: sahiplik, ömür, null ve thread koşullarını gevşetmeyin.
    fn value(&self, param: &str) -> f64 {
        let params: &clap_plugin_params = self.extension(CLAP_EXT_PARAMS);
        let mut v = f64::NAN;
        // SAFETY: as CLAP has it.
        assert!(unsafe { (params.get_value.unwrap())(self.plugin, id(param), &mut v) });
        v
    }

    /// The plug-in's state as the host saves it.
    // Regresyon senaryosu save: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
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
    // load: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir. Mevcut veri/kalıcılık sözleşmesini bu erişim adımında koruyun.
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

/// nih-plug's CLAP state: its length (8 bytes, little-endian), then its JSON.
// json_of alt adımı: clap_host regresyonlarının sentetik/izole girdilerini, sınır koşullarını ve beklenen sonuç denetimlerini içerir.
fn json_of(bytes: &[u8]) -> serde_json::Value {
    let n = u64::from_le_bytes(bytes[..8].try_into().unwrap()) as usize;
    serde_json::from_slice(&bytes[8..8 + n]).unwrap()
}

#[test]
// Regresyon senaryosu clap_automation_and_full_session_roundtrip: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Instance::new → a.process → In::Param → a.save. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn clap_automation_and_full_session_roundtrip() {
    let a = Instance::new();
    a.process(&[
        In::Param(0, "detune", 0.73),
        In::Param(0, "cutoff", 0.65),
        In::Param(0, "arp_on", 1.0),
        In::Param(0, "arp_mode", 2.0 / 3.0),
        In::Param(0, "mseg_enable", 1.0),
        In::Param(0, "mseg_p9_on", 1.0),
        In::Param(0, "mseg_p9_x", 0.63),
        In::Param(0, "mseg_n10_out", 0.8),
        In::Param(0, "mseg_n10_wave", 0.5),
    ]);
    let saved = a.save();
    let json = json_of(&saved);
    assert_eq!(json["params"].as_object().unwrap().len(), 161);
    let b = Instance::new();
    assert!(b.load(&saved));
    for id in [
        "detune",
        "cutoff",
        "arp_on",
        "arp_mode",
        "mseg_enable",
        "mseg_p9_on",
        "mseg_p9_x",
        "mseg_n10_out",
        "mseg_n10_wave",
    ] {
        assert!((a.value(id) - b.value(id)).abs() < 0.001, "{id}");
    }
    assert_eq!(a.rescans.load(Ordering::SeqCst), 0);
}
#[test]
// Regresyon senaryosu clap_supersaw_midi_is_sample_timed_and_can_be_silenced: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Instance::new → p.process → In::Param → In::Note. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
// Eşzamanlılık sınırı: bu kilidin hangi thread’de alındığını kontrol edin; audio callback’e yeni bekleyen kilit taşımayın.
fn clap_supersaw_midi_is_sample_timed_and_can_be_silenced() {
    let p = Instance::new();
    p.process(&[In::Param(0, "mode", 1.0 / 6.0), In::Note(96, 60, true)]);
    assert!(p.audio.lock().unwrap()[..96].iter().all(|v| v.abs() < 1e-6));
    let mut peak = 0f32;
    for _ in 0..40 {
        p.process(&[]);
        for v in p.audio.lock().unwrap().iter() {
            peak = peak.max(v.abs());
        }
    }
    assert!(peak > 0.001, "SuperSaw must sound");
    p.process(&[In::Cc(0, 0, 120, 0)]);
    for _ in 0..35 {
        p.process(&[]);
    }
    assert!(p.audio.lock().unwrap().iter().all(|v| v.abs() < 1e-6));
}

#[test]
// Regresyon senaryosu legacy_sessions_reset_new_modulation_even_in_an_existing_instance: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Instance::new → p.save → json_of → as_object_mut. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn legacy_sessions_reset_new_modulation_even_in_an_existing_instance() {
    let p = Instance::new();
    let saved = p.save();
    let mut json = json_of(&saved);
    json["params"]
        .as_object_mut()
        .unwrap()
        .retain(|id, _| !id.starts_with("mseg_") && id != "overload");
    let payload = serde_json::to_vec(&json).unwrap();
    let mut old = (payload.len() as u64).to_le_bytes().to_vec();
    old.extend(payload);
    p.process(&[
        In::Param(0, "mseg_enable", 1.),
        In::Param(0, "mseg_target", 1.),
        In::Param(0, "mseg_amount", 1.),
        In::Param(0, "overload", 0.8),
    ]);
    assert!(p.value("mseg_enable") > 0.5);
    assert!(p.load(&old));
    assert_eq!(p.value("overload"), 0.);
    assert_eq!(p.value("mseg_enable"), 0.);
    assert_eq!(p.value("mseg_target"), 0.);
}

#[test]
// Regresyon senaryosu legacy_six_target_sessions_migrate_to_the_three_targets: bu girdide beklenen davranışı ve hata/sınır koşullarını doğrular; başarısız assert ilk araştırma noktasıdır.
// Gövdedeki başlıca çağrılar: Instance::new → p.save → json_of → base.clone. Ayrıntı/sıra için aşağıdaki çağrı yerlerini izleyin.
fn legacy_six_target_sessions_migrate_to_the_three_targets() {
    let p = Instance::new();
    let saved = p.save();
    let base = json_of(&saved);
    for (old, new) in [(0., 0.), (1., 0.), (2., 0.5), (3., 0.5), (4., 1.), (5., 1.)] {
        let mut json = base.clone();
        json["version"] = serde_json::json!("0.1.3");
        json["params"]["mseg_target"] = serde_json::json!({"f32":old});
        json["params"]["mseg_amount"] = serde_json::json!({"f32":0.4});
        let data = serde_json::to_vec(&json).unwrap();
        let mut bytes = (data.len() as u64).to_le_bytes().to_vec();
        bytes.extend(data);
        assert!(p.load(&bytes));
        assert_eq!(p.value("mseg_target"), new, "old target {old}");
    }
}
