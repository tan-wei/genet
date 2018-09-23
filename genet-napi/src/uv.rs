use libc;
use std::mem;

pub enum Loop {}

#[repr(C)]
pub struct Async {
    data: *mut libc::c_void,
    buf: [u8; 1280],
}

unsafe impl Send for Async {}
unsafe impl Sync for Async {}

impl Async {
    pub fn new<F: Fn()>(callback: F) -> Async {
        unsafe {
            extern "C" fn async_cb(asyn: *mut Async) {}

            let mut asyn = mem::uninitialized();
            println!(
                "{:?}",
                uv_async_init(uv_default_loop(), &mut asyn, async_cb)
            );
            asyn
        }
    }

    pub fn send(&self) {
        //unsafe { uv_async_send(self) };
    }
}

impl Drop for Async {
    fn drop(&mut self) {
        //unsafe { uv_close(self) };
    }
}

extern "C" {
    fn uv_async_init(
        evloop: *mut Loop,
        async: *mut Async,
        async_cb: extern "C" fn(*mut Async),
    ) -> libc::c_int;
    fn uv_async_send(async: *const Async) -> libc::c_int;
    fn uv_close(async: *mut Async);
    fn uv_default_loop() -> *mut Loop;
}
