use crate::napi::{Env, Status};
use libc;
use std::{os::raw::c_int, ptr};

pub enum Loop {}

#[repr(C)]
struct Inner {
    data: *mut libc::c_void,
    buf: [u8; 128],
}

pub struct Async(*mut Inner);

unsafe impl Send for Async {}
unsafe impl Sync for Async {}

impl Async {
    pub fn new<F: Fn()>(env: &Env, callback: F) -> Async {
        unsafe {
            extern "C" fn async_cb(asyn: *mut Inner) {
                unsafe {
                    let data = (*asyn).data as *mut Box<Fn()>;
                    (*data)();
                }
            }

            let data: *mut Box<Fn()> = Box::into_raw(Box::new(Box::new(callback)));
            let inner = Box::into_raw(Box::new(Inner {
                data: data as *mut libc::c_void,
                buf: [0; 128],
            }));

            let mut evloop: *mut Loop = ptr::null_mut();
            if napi_get_uv_event_loop(env, &mut evloop) != Status::Ok {
                panic!("napi_get_uv_event_loop failed");
            }

            uv_async_init(evloop, inner, async_cb);
            Async(inner)
        }
    }

    pub fn send(&self) {
        unsafe { uv_async_send(self.0) };
    }
}

impl Drop for Async {
    fn drop(&mut self) {
        extern "C" fn close_cb(asyn: *mut Inner) {
            unsafe { Box::from_raw(asyn) };
        }
        unsafe { uv_close(self.0, close_cb) };
    }
}

extern "C" {
    fn uv_async_init(
        evloop: *mut Loop,
        async_data: *mut Inner,
        async_cb: extern "C" fn(*mut Inner),
    ) -> c_int;
    fn uv_async_send(async_data: *const Inner) -> c_int;
    fn uv_close(async_data: *mut Inner, close_cb: extern "C" fn(*mut Inner));

    fn napi_get_uv_event_loop(env: *const Env, evloop: *mut *mut Loop) -> Status;
}
