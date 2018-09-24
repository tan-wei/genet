use libc;

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
    pub fn new<F: Fn()>(callback: F) -> Async {
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
            uv_async_init(uv_default_loop(), inner, async_cb);
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
        async: *mut Inner,
        async_cb: extern "C" fn(*mut Inner),
    ) -> libc::c_int;
    fn uv_async_send(async: *const Inner) -> libc::c_int;
    fn uv_close(async: *mut Inner, close_cb: extern "C" fn(*mut Inner));
    fn uv_default_loop() -> *mut Loop;
}
