extern crate byteorder;
extern crate libc;
#[macro_use]
extern crate plugkit;

use std::io::{BufWriter, Result, Write};
use std::fs::File;
use std::path::Path;
use byteorder::{LittleEndian, WriteBytesExt};
use plugkit::file::{Exporter, RawFrame};
use plugkit::context::Context;
use plugkit::variant::Value;

pub struct PcapExporter {}

impl Exporter for PcapExporter {
    fn is_supported(_ctx: &mut Context, path: &Path) -> bool {
        if let Some(ext) = path.extension() {
            ext == "pcap"
        } else {
            false
        }
    }

    fn start(ctx: &mut Context, path: &Path, cb: &Fn(&mut Context) -> &[RawFrame]) -> Result<()> {
        let ext = path.extension();
        if ext.is_none() || ext.unwrap() != "pcap" {}
        let file = File::create(path)?;
        let mut wtr = BufWriter::new(file);
        wtr.write_all(&[0x4d, 0x3c, 0xb2, 0xa1])?;

        let mut header = false;
        let snaplen: u32 = ctx.get_config("_.pcap.snaplen").get();

        loop {
            let frames = cb(ctx);
            if frames.len() == 0 {
                break;
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
                let incl_len = f.payload().len();
                let orig_len = f.actlen();
                wtr.write_u32::<LittleEndian>(ts_sec as u32)?;
                wtr.write_u32::<LittleEndian>(ts_usec as u32)?;
                wtr.write_u32::<LittleEndian>(incl_len as u32)?;
                wtr.write_u32::<LittleEndian>(orig_len as u32)?;
                wtr.write_all(f.payload())?;
            }
        }
        Ok(())
    }
}

plugkit_module!({});
plugkit_api_file_export!(PcapExporter);
