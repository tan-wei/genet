use libc;
use std::{ffi::CString, mem, ptr};

type Result<T> = ::std::result::Result<T, Status>;

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum Status {
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

pub enum Env {}

impl Env {
    pub fn create_double<'env>(&'env mut self, value: f64) -> Result<&'env mut Value> {
        unsafe {
            let mut result: *mut Value = mem::uninitialized();
            match napi_create_double(self, value, &mut result) {
                Status::NapiOk => Ok(&mut *result),
                s => Err(s),
            }
        }
    }
}

pub enum Value {}

extern "C" {
    fn napi_create_double(env: *mut Env, value: f64, result: *mut *mut Value) -> Status;
}
