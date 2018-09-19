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

pub enum Env {}

impl Env {
    pub fn create_double<'env>(&'env mut self, value: u64) -> Result<&'env Value> {
        unsafe {
            let mut result: *mut Value = mem::uninitialized();
            match napi_create_double(self, value, &mut result) {
            	Status::NapiOk => Ok(&*result),
            	s => Err(s)
            }
        }
    }
}

pub enum Value {}

extern "C" {
    pub fn napi_create_double(env: *mut Env, value: u64, result: *mut *mut Value) -> Status;
}