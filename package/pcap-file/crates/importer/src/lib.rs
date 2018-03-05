#[macro_use]
extern crate plugkit;
extern crate byteorder;
extern crate libc;

use std::io::{Result, Error, ErrorKind, Read, BufReader};
use std::fs::File;
use std::path::Path;
use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use plugkit::file::{Importer, RawFrame};
use plugkit::context::Context;

pub struct PcapImporter {}

impl Importer for PcapImporter {
    fn is_supported(_ctx: &mut Context, path: &Path) -> bool {
        if let Some(ext) = path.extension() {
            ext == "pcap"
        } else {
            false
        }
    }

    fn start(ctx: &mut Context, path: &Path, dst: &mut [RawFrame], cb: &Fn(&mut Context, usize, f64)) -> Result<()> {
        let file = File::open(path)?;
        let mut rdr = BufReader::new(file);
        let magic_number = rdr.read_u32::<BigEndian>()?;

        let (le, nsec) = match magic_number {
            0xd4c3b2a1 => (true, false),
            0xa1b2c3d4 => (false, false),
            0x4d3cb2a1 => (true, true),
            0xa1b23c4d => (false, true),
            _ => return Err(Error::new(ErrorKind::InvalidData, "wrong magic number"))
        };

        let (
            _ver_major,
            _var_minor,
            _thiszone,
            _sigfigs,
            _snaplen,
            network
        ) = if le {
            (
                rdr.read_u16::<LittleEndian>()?,
                rdr.read_u16::<LittleEndian>()?,
                rdr.read_i32::<LittleEndian>()?,
                rdr.read_u32::<LittleEndian>()?,
                rdr.read_u32::<LittleEndian>()?,
                rdr.read_u32::<LittleEndian>()?
            )
        } else {
            (
                rdr.read_u16::<BigEndian>()?,
                rdr.read_u16::<BigEndian>()?,
                rdr.read_i32::<BigEndian>()?,
                rdr.read_u32::<BigEndian>()?,
                rdr.read_u32::<BigEndian>()?,
                rdr.read_u32::<BigEndian>()?
            )
        };

        loop {
            let mut cnt = 0;
            let result = (|| -> Result<()> {
                for idx in 0..dst.len() {
                    let (
                        ts_sec,
                        mut ts_usec,
                        inc_len,
                        orig_len
                    ) = if le {
                        (
                            rdr.read_u32::<LittleEndian>()?,
                            rdr.read_u32::<LittleEndian>()?,
                            rdr.read_u32::<LittleEndian>()?,
                            rdr.read_u32::<LittleEndian>()?
                        )
                    } else {
                        (
                            rdr.read_u32::<BigEndian>()?,
                            rdr.read_u32::<BigEndian>()?,
                            rdr.read_u32::<BigEndian>()?,
                            rdr.read_u32::<BigEndian>()?
                        )
                    };

                    if !nsec {
                        ts_usec *= 1000;
                    }

                    let mut vec = Vec::<u8>::with_capacity(inc_len as usize);
                    unsafe {
                        vec.set_len(inc_len as usize);
                    }
                    rdr.read_exact(&mut vec)?;

                    let frame = &mut dst[idx];
                    frame.set_link(network);
                    frame.set_actlen(orig_len as usize);
                    frame.set_payload_and_forget(vec.into_boxed_slice());
                    frame.set_ts((ts_sec as i64, ts_usec as i64));
                    cnt += 1;
                }
                Ok(())
            })();
            cb(ctx, cnt, 0.5);
            if result.is_err() {
                break
            }
        }
        Ok(())
    }
}

plugkit_module!({});
plugkit_api_file_import!(PcapImporter);
