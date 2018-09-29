extern crate libc;
extern crate libloading;

#[macro_use]
extern crate serde_derive;

extern crate serde;

#[cfg(target_os = "linux")]
extern crate capabilities;

use std::{
    sync::mpsc::{channel, Receiver, Sender},
    thread,
};

const PCAP_IF_LOOPBACK: u32 = 0x0000_0001;
const PCAP_ERRBUF_SIZE: usize = 256;

#[derive(Debug, Serialize, Deserialize)]
pub struct Header {
    pub datalen: u32,
    pub actlen: u32,
    pub ts_sec: u32,
    pub ts_usec: u32,
}

#[derive(Debug)]
pub enum Error {
    NoPermission,
    DLLNotFound,
    DLLFuncNotFound,
    OpenFailed(String),
}

pub type FrameReceiver = Receiver<(Header, Box<[u8]>)>;

#[derive(Debug)]
pub struct Pcap {
    syms: ffi::Symbols,
    handles: Vec<*mut ffi::Pcap>,
}

impl Pcap {
    pub fn new() -> Result<Pcap, Error> {
        if platform::check_permission() {
            let syms = ffi::Symbols::new()?;
            let pcap = Pcap {
                syms,
                handles: Vec::new(),
            };
            if pcap.test_permission() {
                return Ok(pcap);
            }
        }
        Err(Error::NoPermission)
    }

    fn test_permission(&self) -> bool {
        use ffi::*;
        use std::ptr;
        let mut front: *mut PcapIf = ptr::null_mut();
        unsafe {
            if (self.syms.pcap_findalldevs)(&mut front as *mut *mut PcapIf) < 0 || front.is_null() {
                return false;
            }
            (self.syms.pcap_freealldevs)(front);
        }
        true
    }

    pub fn start(&mut self, ifs: &str, snaplen: u32) -> Result<FrameReceiver, Error> {
        use std::{ffi::CString, slice};
        let (send, recv) = channel();
        let ifs = CString::new(ifs).unwrap();

        #[derive(Clone)]
        struct PcapHolder {
            pcap: *mut ffi::Pcap,
            syms: ffi::Symbols,
            sender: Sender<(Header, Box<[u8]>)>,
        }

        unsafe impl Send for PcapHolder {}

        let errbuf = [0u8; PCAP_ERRBUF_SIZE];
        unsafe {
            let pcap = (self.syms.pcap_open_live)(
                ifs.as_ptr(),
                snaplen as i32,
                0,
                1,
                errbuf.as_ptr() as *mut libc::c_char,
            );
            if pcap.is_null() {
                let mut msg = String::new();
                if let Some(pos) = errbuf.iter().position(|c| *c == 0) {
                    if let Ok(s) = String::from_utf8(errbuf[..pos].to_vec()) {
                        msg = s;
                    }
                }
                return Err(Error::OpenFailed(msg));
            }

            self.handles.push(pcap);

            extern "C" fn handler(
                user: *mut libc::c_uchar,
                h: *const ffi::PcapPkthdr,
                data: *const libc::c_uchar,
            ) {
                unsafe {
                    let holder = &*(user as *const PcapHolder);
                    let h = &*h;
                    let data = slice::from_raw_parts(data, h.caplen as usize);
                    let header = Header {
                        datalen: data.len() as u32,
                        actlen: h.len,
                        ts_sec: h.ts.tv_sec as u32,
                        ts_usec: h.ts.tv_usec as u32,
                    };
                    if holder.sender.send((header, data.into())).is_err() {
                        (holder.syms.pcap_breakloop)(holder.pcap);
                    }
                }
            }

            let holder = PcapHolder {
                pcap,
                syms: self.syms.clone(),
                sender: send,
            };
            thread::spawn(move || {
                (holder.syms.pcap_loop)(
                    holder.pcap,
                    0,
                    handler,
                    &holder as *const PcapHolder as *mut u8,
                );
                (holder.syms.pcap_close)(holder.pcap);
            });
        }
        Ok(recv)
    }

    pub fn devices(&self) -> Option<Vec<Device>> {
        use ffi::*;
        use libc;
        use std::ptr;
        let mut front: *mut PcapIf = ptr::null_mut();

        let mut devs = Vec::new();
        unsafe {
            if (self.syms.pcap_findalldevs)(&mut front as *mut *mut PcapIf) < 0 {
                return None;
            }

            let errbuf = [0i8; PCAP_ERRBUF_SIZE];

            let mut cur = front;
            while !cur.is_null() {
                let id = getstr((*cur).name);
                let description = getstr((*cur).description);
                let loopback = (*cur).flags & PCAP_IF_LOOPBACK != 0;
                let link = {
                    let pcap = (self.syms.pcap_open_live)(
                        (*cur).name,
                        2048,
                        0,
                        0,
                        errbuf.as_ptr() as *mut libc::c_char,
                    );
                    let mut link = 0;
                    if !pcap.is_null() {
                        link = (self.syms.pcap_datalink)(pcap);
                        (self.syms.pcap_close)(pcap);
                    }
                    link
                };

                devs.push(Device::new(id.clone(), id, description, link, loopback));
                cur = (*cur).next;
            }

            (self.syms.pcap_freealldevs)(front);
        }

        Some(platform::device_descriptions(devs))
    }
}

impl Drop for Pcap {
    fn drop(&mut self) {
        while let Some(h) = self.handles.pop() {
            unsafe {
                (self.syms.pcap_breakloop)(h);
            }
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Device {
    id: String,
    name: String,
    description: String,
    link: i32,
    loopback: bool,
}

impl Device {
    fn new(id: String, name: String, description: String, link: i32, loopback: bool) -> Device {
        Device {
            id,
            name,
            description,
            link,
            loopback,
        }
    }

    pub fn id(&self) -> &str {
        &self.id
    }

    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn description(&self) -> &str {
        &self.description
    }

    pub fn link(&self) -> i32 {
        self.link
    }

    pub fn loopback(&self) -> bool {
        self.loopback
    }
}

mod platform {
    extern crate libc;
    use super::Device;

    #[cfg(target_os = "macos")]
    pub(crate) fn device_descriptions(devices: Vec<Device>) -> Vec<Device> {
        use super::platform::macos::*;

        fn getstr(string: *const CFString) -> String {
            use std::ffi::CStr;
            const K_CF_STRING_ENCODING_UTF8: u32 = 0x0800_0100;
            const BUFSIZE: libc::c_long = 2048;

            let buf = [0i8; BUFSIZE as usize];
            unsafe {
                CFStringGetCString(
                    string,
                    buf.as_ptr() as *mut libc::c_char,
                    BUFSIZE,
                    K_CF_STRING_ENCODING_UTF8,
                );
                String::from(CStr::from_ptr(buf.as_ptr()).to_str().unwrap_or(""))
            }
        }

        use std::collections::HashMap;
        let mut map = HashMap::new();

        unsafe {
            let array = SCNetworkInterfaceCopyAll();
            let size = CFArrayGetCount(array);
            for i in 0..size {
                let ifs = CFArrayGetValueAtIndex(array, i);
                let id = getstr(SCNetworkInterfaceGetBSDName(ifs));
                let name = getstr(SCNetworkInterfaceGetLocalizedDisplayName(ifs));
                map.insert(id, name);
            }
            CFRelease(array);
        }

        devices
            .into_iter()
            .map(|dev| {
                let name = map.get(&dev.id).unwrap_or(&dev.id).to_string();
                Device::new(dev.id, name, dev.description, dev.link, dev.loopback)
            })
            .collect()
    }

    #[cfg(target_os = "macos")]
    mod macos {
        extern crate libc;

        pub(crate) enum CFString {}
        pub(crate) enum CFArray {}
        pub(crate) enum SCNetworkInterface {}

        #[link(name = "Foundation", kind = "framework")]
        extern "C" {
            pub(crate) fn CFStringGetCString(
                string: *const CFString,
                buffer: *mut libc::c_char,
                bufferSize: libc::c_long,
                encoding: u32,
            ) -> bool;
            pub(crate) fn CFArrayGetCount(ptr: *const CFArray) -> libc::c_long;
            pub(crate) fn CFArrayGetValueAtIndex(
                ptr: *const CFArray,
                index: libc::c_long,
            ) -> *const SCNetworkInterface;
            pub(crate) fn CFRelease(ptr: *const CFArray);
        }

        #[link(name = "SystemConfiguration", kind = "framework")]
        extern "C" {
            pub(crate) fn SCNetworkInterfaceCopyAll() -> *const CFArray;
            pub(crate) fn SCNetworkInterfaceGetBSDName(
                ifs: *const SCNetworkInterface,
            ) -> *const CFString;
            pub(crate) fn SCNetworkInterfaceGetLocalizedDisplayName(
                ifs: *const SCNetworkInterface,
            ) -> *const CFString;
        }
    }

    #[cfg(target_os = "windows")]
    pub(crate) fn device_descriptions(devices: Vec<Device>) -> Vec<Device> {
        use super::platform::windows::*;

        use std::{mem, ptr};
        const NO_ERROR: u32 = 0;
        const ERROR_INSUFFICIENT_BUFFER: u32 = 122;
        const GUID_LEN: usize = 38;

        let mut size = 0u32;
        let mut table: *mut MibIftable = ptr::null_mut();

        use std::collections::HashMap;
        let mut map = HashMap::new();

        unsafe {
            if GetIfTable(
                ptr::null_mut::<MibIftable>(),
                &mut size as *mut libc::c_ulong,
                false,
            ) == ERROR_INSUFFICIENT_BUFFER
            {
                table = mem::transmute(libc::malloc(size as libc::size_t));
            }

            if GetIfTable(table, &mut size as *mut libc::c_ulong, false) == NO_ERROR {
                for i in 1..((*table).dw_num_entries + 1) {
                    let mut row = MibIfrow {
                        wsz_name: [0; MAX_INTERFACE_NAME_LEN],
                        dw_index: i,
                        _padding: [0; 84],
                        dw_descr_len: 0,
                        b_descr: [0; 256],
                    };
                    if GetIfEntry(&mut row as *mut MibIfrow) == NO_ERROR {
                        if let Some(pos) = row.wsz_name.iter().position(|c| *c == 0) {
                            if let Ok(name) =
                                String::from_utf16(&row.wsz_name[(pos - GUID_LEN)..pos])
                            {
                                if let Ok(desc) = String::from_utf8(
                                    row.b_descr[..(row.dw_descr_len - 1) as usize].to_vec(),
                                ) {
                                    map.insert(name, desc);
                                }
                            }
                        }
                    }
                }
            }
            libc::free(mem::transmute(table));
        }

        devices
            .into_iter()
            .map(|dev| {
                let prefix = dev.id.len() - GUID_LEN;
                let guid = dev.id.chars().skip(prefix).collect();
                let name = map.get(&guid).unwrap_or(&guid).to_string();
                Device::new(dev.id, name, dev.description, dev.link, dev.loopback)
            })
            .collect()
    }

    #[cfg(target_os = "windows")]
    mod windows {
        extern crate libc;

        pub(crate) const MAX_INTERFACE_NAME_LEN: usize = 256;
        pub(crate) const MAXLEN_IFDESCR: usize = 256;

        #[repr(C)]
        pub(crate) struct MibIfrow {
            pub wsz_name: [u16; MAX_INTERFACE_NAME_LEN],
            pub dw_index: u32,
            pub _padding: [u8; 84],
            pub dw_descr_len: u32,
            pub b_descr: [u8; MAXLEN_IFDESCR],
        }

        #[repr(C)]
        pub(crate) struct MibIftable {
            pub dw_num_entries: u32,
        }

        #[link(name = "iphlpapi")]
        #[allow(non_snake_case)]
        extern "system" {
            pub(crate) fn GetIfTable(
                table: *mut MibIftable,
                size: *mut libc::c_ulong,
                order: bool,
            ) -> u32;
            pub(crate) fn GetIfEntry(row: *mut MibIfrow) -> u32;
        }
    }

    #[cfg(not(any(target_os = "macos", target_os = "windows")))]
    pub(crate) fn device_descriptions(devices: Vec<Device>) -> Vec<Device> {
        devices
    }

    #[cfg(target_os = "macos")]
    pub(crate) fn check_permission() -> bool {
        use std::{
            fs,
            io::{Error, ErrorKind},
            os::unix::fs::MetadataExt,
        };

        fn check_dev() -> Result<(), Error> {
            for entry in fs::read_dir("/dev")? {
                let entry = entry?;
                let path = entry.path();
                if path.to_str().unwrap().starts_with("/dev/bpf") {
                    let attr = fs::metadata(path)?;
                    const S_IRGRP: u32 = 0o000_040; /* [XSI] R for group */
                    if attr.mode() & S_IRGRP == 0 || attr.gid() == 0 {
                        return Err(Error::new(ErrorKind::PermissionDenied, ""));
                    }
                }
            }
            Ok(())
        }
        check_dev().is_ok()
    }

    #[cfg(target_os = "linux")]
    pub(crate) fn check_permission() -> bool {
        use capabilities::{Capabilities, Capability, Flag};

        if let Ok(current) = Capabilities::from_current_proc() {
            current.check(Capability::CAP_NET_ADMIN, Flag::Effective)
                && current.check(Capability::CAP_NET_ADMIN, Flag::Permitted)
                && current.check(Capability::CAP_NET_RAW, Flag::Effective)
                && current.check(Capability::CAP_NET_RAW, Flag::Permitted)
        } else {
            false
        }
    }

    #[cfg(not(any(target_os = "macos", target_os = "linux")))]
    pub(crate) fn check_permission() -> bool {
        true
    }
}

mod ffi {
    extern crate libc;
    use std::sync::Arc;

    pub(crate) fn getstr(ptr: *mut libc::c_char) -> String {
        use std::ffi::CStr;
        if ptr.is_null() {
            String::new()
        } else {
            unsafe { String::from(CStr::from_ptr(ptr).to_str().unwrap_or("")) }
        }
    }

    #[derive(Debug, Clone)]
    pub(crate) struct Symbols {
        lib: Option<Arc<super::libloading::Library>>,
        pub pcap_findalldevs: unsafe extern "C" fn(alldevsp: *mut *mut PcapIf) -> libc::c_int,
        pub pcap_freealldevs: unsafe extern "C" fn(alldevsp: *mut PcapIf),
        pub pcap_open_live: unsafe extern "C" fn(
            device: *const libc::c_char,
            snaplen: libc::c_int,
            promisc: libc::c_int,
            to_ms: libc::c_int,
            errbuf: *mut libc::c_char,
        ) -> *mut Pcap,
        pub pcap_datalink: unsafe extern "C" fn(pcap: *mut Pcap) -> libc::c_int,
        pub pcap_loop: unsafe extern "C" fn(
            pcap: *mut Pcap,
            cnt: libc::c_int,
            callback: PcapHandler,
            user: *mut libc::c_uchar,
        ) -> libc::c_int,
        pub pcap_breakloop: unsafe extern "C" fn(pcap: *mut Pcap),
        pub pcap_close: unsafe extern "C" fn(pcap: *mut Pcap),
    }

    impl Symbols {
        #[cfg(not(windows))]
        pub(crate) fn new() -> Result<Symbols, super::Error> {
            Ok(Symbols {
                lib: None,
                pcap_findalldevs,
                pcap_freealldevs,
                pcap_open_live,
                pcap_datalink,
                pcap_loop,
                pcap_breakloop,
                pcap_close,
            })
        }

        #[cfg(windows)]
        pub(crate) fn new() -> Result<Symbols, super::Error> {
            use super::libloading;
            use std::ops::Deref;
            let lib =
                libloading::Library::new("Wpcap.dll").map_err(|_| super::Error::DLLNotFound)?;

            let pcap_findalldevs;
            let pcap_freealldevs;
            let pcap_open_live;
            let pcap_datalink;
            let pcap_loop;
            let pcap_breakloop;
            let pcap_close;

            {
                let pcap_findalldevs_: libloading::Symbol<
                    unsafe extern "C" fn(alldevsp: *mut *mut PcapIf) -> libc::c_int,
                >;
                let pcap_freealldevs_: libloading::Symbol<
                    unsafe extern "C" fn(alldevsp: *mut PcapIf),
                >;
                let pcap_open_live_: libloading::Symbol<
                    unsafe extern "C" fn(
                        device: *const libc::c_char,
                        snaplen: libc::c_int,
                        promisc: libc::c_int,
                        to_ms: libc::c_int,
                        errbuf: *mut libc::c_char,
                    ) -> *mut Pcap,
                >;
                let pcap_datalink_: libloading::Symbol<
                    unsafe extern "C" fn(pcap: *mut Pcap) -> libc::c_int,
                >;
                let pcap_loop_: libloading::Symbol<
                    unsafe extern "C" fn(
                        pcap: *mut Pcap,
                        cnt: libc::c_int,
                        callback: PcapHandler,
                        user: *mut libc::c_uchar,
                    ) -> libc::c_int,
                >;
                let pcap_breakloop_: libloading::Symbol<
                    unsafe extern "C" fn(pcap: *mut Pcap),
                >;
                let pcap_close_: libloading::Symbol<
                    unsafe extern "C" fn(pcap: *mut Pcap),
                >;

                unsafe {
                    pcap_findalldevs_ = lib
                        .get(b"pcap_findalldevs")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_freealldevs_ = lib
                        .get(b"pcap_freealldevs")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_open_live_ = lib
                        .get(b"pcap_open_live")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_datalink_ = lib
                        .get(b"pcap_datalink")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_loop_ = lib
                        .get(b"pcap_loop")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_close_ = lib
                        .get(b"pcap_close")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                    pcap_breakloop_ = lib
                        .get(b"pcap_breakloop")
                        .map_err(|_| super::Error::DLLFuncNotFound)?;
                }

                pcap_findalldevs = *pcap_findalldevs_.deref();
                pcap_freealldevs = *pcap_freealldevs_.deref();
                pcap_open_live = *pcap_open_live_.deref();
                pcap_datalink = *pcap_datalink_.deref();
                pcap_loop = *pcap_loop_.deref();
                pcap_breakloop = *pcap_breakloop_.deref();
                pcap_close = *pcap_close_.deref();
            }

            Ok(Symbols {
                lib: Some(Arc::new(lib)),
                pcap_findalldevs,
                pcap_freealldevs,
                pcap_open_live,
                pcap_datalink,
                pcap_loop,
                pcap_breakloop,
                pcap_close,
            })
        }
    }

    pub(crate) enum Pcap {}
    pub(crate) enum SockAddr {}

    #[repr(C)]
    #[derive(Debug)]
    pub(crate) struct PcapAddr {
        pub next: *mut PcapAddr,
        pub addr: *mut SockAddr,
        pub netmask: *mut SockAddr,
        pub broadaddr: *mut SockAddr,
        pub dstaddr: *mut SockAddr,
    }

    #[repr(C)]
    #[derive(Debug)]
    pub(crate) struct PcapIf {
        pub next: *mut PcapIf,
        pub name: *mut libc::c_char,
        pub description: *mut libc::c_char,
        pub addresses: *mut PcapAddr,
        pub flags: u32,
    }

    #[repr(C)]
    pub(crate) struct PcapPkthdr {
        pub ts: libc::timeval,
        pub caplen: u32,
        pub len: u32,
        pub comment: *mut libc::c_char,
    }

    pub(crate) type PcapHandler =
        extern "C" fn(*mut libc::c_uchar, *const PcapPkthdr, *const libc::c_uchar);

    #[cfg(not(target_os = "windows"))]
    #[link(name = "pcap")]
    extern "C" {
        fn pcap_findalldevs(alldevsp: *mut *mut PcapIf) -> libc::c_int;
        fn pcap_freealldevs(alldevsp: *mut PcapIf);
        fn pcap_open_live(
            device: *const libc::c_char,
            snaplen: libc::c_int,
            promisc: libc::c_int,
            to_ms: libc::c_int,
            errbuf: *mut libc::c_char,
        ) -> *mut Pcap;
        fn pcap_datalink(pcap: *mut Pcap) -> libc::c_int;
        fn pcap_loop(
            pcap: *mut Pcap,
            cnt: libc::c_int,
            callback: PcapHandler,
            user: *mut libc::c_uchar,
        ) -> libc::c_int;
        fn pcap_breakloop(pcap: *mut Pcap);
        fn pcap_close(pcap: *mut Pcap);
    }
}
