//! Macros for defining plugin functions.

/// Define file importer
#[macro_export]
macro_rules! plugkit_api_file_import {
    ( $x:ident ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_file_importer_is_supported(c: *mut Context, p: *const libc::c_char) -> bool {
            use std::ffi::CStr;
            use std::str;
            use std::path::Path;
            let path = unsafe {
                let slice = CStr::from_ptr(p);
                Path::new(str::from_utf8_unchecked(slice.to_bytes()))
            };
            let ctx = unsafe { &mut *c };
            $x::is_supported(ctx, path)
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_file_importer_start(c: *mut Context, p: *const libc::c_char, d: *mut RawFrame,
                s: libc::size_t, callback: extern "C" fn (*mut Context, libc::size_t, f64)) -> plugkit::file::Status {
            use std::ffi::CStr;
            use std::{str,slice};
            use std::path::Path;
            use plugkit::file::Status;
            let path = unsafe {
                let slice = CStr::from_ptr(p);
                Path::new(str::from_utf8_unchecked(slice.to_bytes()))
            };
            let dst = unsafe {
                slice::from_raw_parts_mut(d, s as usize)
            };
            let ctx = unsafe { &mut *c };

            let result = $x::start(ctx, path, dst, &|ctx, len, prog| {
                callback(ctx as *mut Context, len, prog);
            });
            callback(c, 0, 1.0);
            if result.is_err() {
                Status::Error
            } else {
                Status::Done
            }
        }
    };
}

/// Define file exporter
#[macro_export]
macro_rules! plugkit_api_file_export {
    ( $x:ident ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_file_exporter_is_supported(c: *mut Context, p: *const libc::c_char) -> bool {
            use std::ffi::CStr;
            use std::str;
            use std::path::Path;
            let path = unsafe {
                let slice = CStr::from_ptr(p);
                Path::new(str::from_utf8_unchecked(slice.to_bytes()))
            };
            let ctx = unsafe { &mut *c };
            $x::is_supported(ctx, path)
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_file_exporter_start(c: *mut Context, p: *const libc::c_char,
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
            let result = $x::start(ctx, path, &|ctx| {
                let mut len : libc::size_t = 0;
                unsafe {
                    let ptr = &*callback(ctx as *mut Context, &mut len as *mut libc::size_t);
                    slice::from_raw_parts(ptr, len as usize)
                }
            });
            if result.is_err() {
                Status::Error
            } else {
                Status::Done
            }
        }
    };
}

/// Define layer hints
#[macro_export]
macro_rules! plugkit_api_layer_hints {
    ( $( $x:expr ), * ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_layer_hints(index: u32) -> u32 {
            let mut temp_vec = Vec::new();
            $(
                temp_vec.push($x);
            )*
            temp_vec.push(0);
            temp_vec[index as usize]
        }
    };
}

/// Define worker
#[macro_export]
macro_rules! plugkit_api_worker {
    ( $x: ty, $y:expr ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_create_worker(_ctx: *const (), _diss: *const ()) -> *mut() {
            Box::into_raw(Box::new($y)) as *mut()
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_destroy_worker(_ctx: *const (), _diss: *const (), worker: *mut()) {
            unsafe {
                Box::from_raw(worker as *mut $x);
            }
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_examine(ctx: *mut (), _diss: *const (), worker: *mut(), layer: *const ()) -> u32 {
            use std::panic;
            use plugkit::context::Context;
            use plugkit::layer::Layer;
            unsafe {
                let w: &mut $x = &mut *(worker as *mut $x);
                let c: &mut Context =
                    &mut *(ctx as *mut Context);
                let l: &Layer = &*(layer as *const Layer);
                let result = panic::catch_unwind(panic::AssertUnwindSafe(|| {
                    w.examine(c, l)
                }));
                match result {
                    Ok(conf) => conf as u32,
                    Err(e) => {
                        println!("{:?}", e);
                        0
                    },
                }
            }
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_analyze(ctx: *mut (), _diss: *const (), worker: *mut(), layer: *mut ()) {
            use std::panic;
            use plugkit::context::Context;
            use plugkit::layer::Layer;
            unsafe {
                let w: &mut $x = &mut *(worker as *mut $x);
                let c: &mut Context =
                    &mut *(ctx as *mut Context);
                let l: &mut Layer = &mut *(layer as *mut Layer);
                let result = panic::catch_unwind(panic::AssertUnwindSafe(|| {
                    w.analyze(c, l)
                }));
                match result {
                    Ok(_) => {},
                    Err(e) => {
                        println!("{:?}", e);
                    },
                };
            }
        }
    };
}
