#[macro_use]
extern crate plugkit;
extern crate byteorder;
extern crate libc;

use std::io::{Result, Error, ErrorKind, Write, BufWriter};
use std::fs::File;
use std::path::Path;
use byteorder::{LittleEndian, WriteBytesExt};
use plugkit::file::{Exporter, RawFrame};
use plugkit::context::Context;
use plugkit::variant::Value;

pub struct PcapExporter {}

impl Exporter for PcapExporter {
    fn start(ctx: &mut Context, path: &Path, cb: &Fn(&mut Context) -> &[RawFrame]) -> Result<()> {
        let ext = path.extension();
        if ext.is_none() || ext.unwrap() != "pcap" {
            return Err(Error::new(ErrorKind::InvalidInput, "unsupported"))
        }
        let file = File::create(path)?;
        let mut wtr = BufWriter::new(file);
        wtr.write_all(&[0x4d, 0x3c, 0xb2, 0xa1])?;

        let mut header = false;
        let snaplen : u32 = ctx.get_option("_.pcap.snaplen").get();

        loop {
            let frames = cb(ctx);
            if frames.len() == 0 {
                break
            }

            if !header {
                header = true;

                let var_major = 2u16;
                let var_minor = 4u16;
                let thiszone = 0i32;
                let sigfigs = 0u32;
                let network = frames[0].link();
                wtr.write_u16::<LittleEndian>(var_major)?;
                wtr.write_u16::<LittleEndian>(var_minor)?;
                wtr.write_i32::<LittleEndian>(thiszone)?;
                wtr.write_u32::<LittleEndian>(sigfigs)?;
                wtr.write_u32::<LittleEndian>(snaplen)?;
                wtr.write_u32::<LittleEndian>(network)?;
            }

            for f in frames {
                let (ts_sec, ts_usec) = f.ts();
                let incl_len = f.data().len();
                let orig_len = f.actlen();
                wtr.write_u32::<LittleEndian>(ts_sec as u32)?;
                wtr.write_u32::<LittleEndian>(ts_usec as u32)?;
                wtr.write_u32::<LittleEndian>(incl_len as u32)?;
                wtr.write_u32::<LittleEndian>(orig_len as u32)?;
                wtr.write_all(f.data())?;
            }
        }
        Ok(())
    }
}

#[no_mangle]
pub extern "C" fn plugkit_v1_file_export(c: *mut Context, p: *const libc::c_char,
    callback: extern "C" fn (*mut Context, *mut libc::size_t) -> *const RawFrame) -> plugkit::file::Status {
    use std::ffi::CStr;
    use std::{str,slice};
    use std::path::Path;
    use plugkit::file::Status;
    let path = unsafe {
        let slice = CStr::from_ptr(p);
        Path::new(str::from_utf8_unchecked(slice.to_bytes()))
    };
    let ctx = unsafe { &mut *c };
    let result = PcapExporter::start(ctx, path, &|ctx| {
        let mut len : libc::size_t = 0;
        unsafe {
            let ptr = &*callback(ctx as *mut Context, &mut len as *mut libc::size_t);
            slice::from_raw_parts(ptr, len as usize)
        }
    });
    if let Err(e) = result {
        match e.kind() {
            ErrorKind::InvalidInput => Status::Unsupported,
            _ => Status::Error,
        }
    } else {
        Status::Done
    }
}

plugkit_module!({});
