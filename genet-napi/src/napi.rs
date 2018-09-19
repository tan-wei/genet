use libc;
use std::mem;

type Result<T> = ::std::result::Result<T, Status>;

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
enum Status {
    NapiOk,
    NapiInvalidArg,
    NapiObjectExpected,
    NapiStringExpected,
    NapiNameExpected,
    NapiFunctionExpected,
    NapiNumberExpected,
    NapiBooleanExpected,
    NapiArrayExpected,
    NapiGenericFailure,
    NapiPendingException,
    NapiCancelled,
    NapiEscapeCalledTwice,
    NapiHandleScopeMismatch,
    NapiCallbackScopeMismatch,
    NapiQueueFull,
    NapiClosing,
    NapiBigintExpected,
}

const NAPI_MODULE_VERSION: libc::c_int = 1;

#[repr(C)]
struct Module {
    version: libc::c_int,
    flags: libc::c_uint,
    filename: *const libc::c_char,
    register_func: extern "C" fn(env: *mut Env, exports: *mut Value) -> *mut Value,
    modname: *const libc::c_char,
    private: *const libc::c_void,
    reserved: [*const libc::c_void; 4],
}

impl Module {
    fn new<'env, F: Fn(&'env mut Env, &'env mut Value)>(init: F) -> Module {
        mem::forget(init);
        unsafe { mem::uninitialized() }
    }
}

pub enum Env {}

impl Env {
    pub fn create_double<'env>(&'env mut self, value: u64) -> Result<&'env Value> {
        unsafe {
            let mut result: *mut Value = mem::uninitialized();
            match napi_create_double(self, value, &mut result) {
                Status::NapiOk => Ok(&*result),
                s => Err(s),
            }
        }
    }
}

pub enum Value {}

extern "C" {
    fn napi_module_register(module: *mut Module);
    fn napi_create_double(env: *mut Env, value: u64, result: *mut *mut Value) -> Status;
}

#[allow(improper_ctypes)]
#[cfg_attr(target_os = "linux", link_section = ".ctors")]
#[cfg_attr(target_os = "macos", link_section = "__DATA,__mod_init_func")]
#[cfg_attr(target_os = "windows", link_section = ".CRT$XCU")]
pub static NAPI_C_CTOR: extern "C" fn() = {
    extern "C" fn register_module() {
        let mut module = Module::new(|_, _| {});
        unsafe { napi_module_register(&mut module) };
        mem::forget(module);
    }
    register_module
};
