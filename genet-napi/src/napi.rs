use genet_abi::slice::ByteSlice;
use libc;
use std::{ffi::CString, mem, ptr};

pub type Result<T> = ::std::result::Result<T, Status>;

bitflags! {
    pub struct PropertyAttributes: u32 {
        const Default = 0;
        const Writable = 1;
        const Enumerable = 1 << 1;
        const Configurable = 1 << 2;
        const Static = 1 << 10;
    }
}

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum Status {
    Ok,
    InvalidArg,
    ObjectExpected,
    StringExpected,
    NameExpected,
    FunctionExpected,
    NumberExpected,
    BooleanExpected,
    ArrayExpected,
    GenericFailure,
    PendingException,
    Cancelled,
    EscapeCalledTwice,
    HandleScopeMismatch,
    CallbackScopeMismatch,
    QueueFull,
    Closing,
    BigintExpected,
}

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum ValueType {
    Undefined,
    Null,
    Boolean,
    Number,
    String,
    Symbol,
    Object,
    Function,
    External,
    Bigint,
}

pub enum Env {}

impl Env {
    pub fn get_undefined<'env>(&'env self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_undefined(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_null<'env>(&'env self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_null(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_global<'env>(&'env self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_global(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_boolean<'env>(&'env self, value: bool) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_boolean(self, if value { 1 } else { 0 }, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_object<'env>(&'env self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_object(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_string<'env>(&'env self, s: &str) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let status = if s.is_ascii() {
                napi_create_string_latin1(self, s.as_ptr() as *const i8, s.len(), &mut result)
            } else {
                napi_create_string_utf8(self, s.as_ptr() as *const i8, s.len(), &mut result)
            };
            match status {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_function<'env>(
        &'env self,
        name: &str,
        func: fn(&'env Env, &'env CallbackInfo) -> Result<&'env Value>,
    ) -> Result<&'env Value> {
        #[repr(transparent)]
        struct FuncData<'env>(fn(&'env Env, &'env CallbackInfo) -> Result<&'env Value>);
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            extern "C" fn cb(env: *const Env, info: *const CbInfo) -> *const Value {
                unsafe {
                    let env = &*env;
                    let info = env.get_cb_info(&*info).unwrap();
                    let func: FuncData = mem::transmute(info.data);
                    match func.0(env, &info) {
                        Ok(v) => v,
                        Err(s) => {
                            let _ = env.throw_error("napi_status", &format!("{:?}", s));
                            env.get_undefined().unwrap()
                        }
                    }
                }
            }
            match napi_create_function(
                self,
                name.as_ptr() as *const i8,
                name.len(),
                cb,
                mem::transmute(FuncData(func)),
                &mut result,
            ) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_double<'env>(&'env self, value: f64) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_double(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_int32<'env>(&'env self, value: i32) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_int32(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_uint32<'env>(&'env self, value: u32) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_uint32(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_int64<'env>(&'env self, value: i64) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_int64(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_array<'env>(&'env self, len: usize) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_array_with_length(self, len, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_symbol<'env>(&'env self, description: &'env Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_symbol(self, description, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_error<'env>(
        &'env self,
        code: &'env Value,
        msg: &'env Value,
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_type_error<'env>(
        &'env self,
        code: &'env Value,
        msg: &'env Value,
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_type_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_range_error<'env>(
        &'env self,
        code: &'env Value,
        msg: &'env Value,
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_range_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_arraybuffer_from_slice<'env>(
        &'env self,
        data: &ByteSlice,
    ) -> Result<&'env Value> {
        unsafe {
            extern "C" fn finalize_cb(_env: *const Env, _data: *mut u8, _hint: *mut u8) {}

            let mut result: *const Value = mem::uninitialized();
            match napi_create_external_arraybuffer(
                self,
                data.as_ptr() as *mut libc::c_void,
                data.len(),
                finalize_cb,
                ptr::null_mut(),
                &mut result,
            ) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_double<'env>(&'env self, value: &'env Value) -> Result<f64> {
        unsafe {
            let mut result: f64 = mem::uninitialized();
            match napi_get_value_double(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_int32<'env>(&'env self, value: &'env Value) -> Result<i32> {
        unsafe {
            let mut result: i32 = mem::uninitialized();
            match napi_get_value_int32(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_uint32<'env>(&'env self, value: &'env Value) -> Result<u32> {
        unsafe {
            let mut result: u32 = mem::uninitialized();
            match napi_get_value_uint32(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_int64<'env>(&'env self, value: &'env Value) -> Result<i64> {
        unsafe {
            let mut result: i64 = mem::uninitialized();
            match napi_get_value_int64(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_bool<'env>(&'env self, value: &'env Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_get_value_bool(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_value_string<'env>(&'env self, value: &'env Value) -> Result<String> {
        unsafe {
            let mut result: libc::size_t = mem::uninitialized();
            match napi_get_value_string_utf8(self, value, ptr::null_mut(), 0, &mut result) {
                Status::Ok => (),
                s => return Err(s),
            };
            if result == 0 {
                return Ok(String::new());
            }
            let mut v: Vec<u8> = Vec::with_capacity(result);
            v.set_len(result - 1);
            match napi_get_value_string_utf8(
                self,
                value,
                v.as_mut_slice().as_mut_ptr() as *mut i8,
                result,
                &mut result,
            ) {
                Status::Ok => Ok(String::from_utf8(v).unwrap()),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_bool<'env>(&'env self, value: &'env Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_bool(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_number<'env>(&'env self, value: &'env Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_number(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_object<'env>(&'env self, value: &'env Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_object(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_string<'env>(&'env self, value: &'env Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_string(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn set_property<'env>(
        &'env self,
        object: &'env Value,
        key: &'env Value,
        value: &'env Value,
    ) -> Result<()> {
        unsafe {
            match napi_set_property(self, object, key, value) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn set_named_property<'env>(
        &'env self,
        object: &'env Value,
        utf8name: &str,
        value: &'env Value,
    ) -> Result<()> {
        unsafe {
            let name = CString::new(utf8name).unwrap();
            match napi_set_named_property(self, object, name.as_ptr(), value) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw<'env>(&'env self, error: &'env Value) -> Result<()> {
        unsafe {
            match napi_throw(self, error) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw_error<'env>(&'env self, code: &str, msg: &str) -> Result<()> {
        unsafe {
            let code = CString::new(code).unwrap();
            let msg = CString::new(msg).unwrap();
            match napi_throw_error(self, code.as_ptr(), msg.as_ptr()) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw_range_error<'env>(&'env self, code: &str, msg: &str) -> Result<()> {
        unsafe {
            let code = CString::new(code).unwrap();
            let msg = CString::new(msg).unwrap();
            match napi_throw_range_error(self, code.as_ptr(), msg.as_ptr()) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn is_error<'env>(&'env self, value: &'env Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_error(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn type_of<'env>(&'env self, value: &'env Value) -> Result<ValueType> {
        unsafe {
            let mut result: ValueType = mem::uninitialized();
            match napi_typeof(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn is_array<'env>(&'env self, value: &'env Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_array(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_array_length<'env>(&'env self, value: &'env Value) -> Result<u32> {
        unsafe {
            let mut result: u32 = mem::uninitialized();
            match napi_get_array_length(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn strict_equals<'env>(&'env self, lhs: &'env Value, rhs: &'env Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_strict_equals(self, lhs, rhs, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_cb_info<'env>(&'env self, cbinfo: &'env CbInfo) -> Result<CallbackInfo<'env>> {
        unsafe {
            const MAX_ARGC: usize = 16;
            let mut argv: [*const Value; MAX_ARGC] = mem::uninitialized();
            let mut argc: libc::size_t = MAX_ARGC;
            let mut this: *const Value = mem::uninitialized();
            let mut data: *const libc::c_void = mem::uninitialized();
            match napi_get_cb_info(
                self,
                cbinfo,
                &mut argc,
                argv.as_mut_ptr(),
                &mut this,
                &mut data,
            ) {
                Status::Ok => Ok(CallbackInfo {
                    argv: argv.into_iter().take(argc).map(|v| &**v).collect(),
                    this: &*this,
                    data,
                }),
                s => Err(s),
            }
        }
    }
}

pub enum Value {}
pub enum CbInfo {}
pub type Callback = extern "C" fn(env: *const Env, info: *const CbInfo) -> *const Value;

pub struct CallbackInfo<'env> {
    argv: Vec<&'env Value>,
    this: &'env Value,
    data: *const libc::c_void,
}

impl<'env> CallbackInfo<'env> {
    pub fn argv(&self) -> &[&'env Value] {
        &self.argv
    }

    pub fn this(&self) -> &'env Value {
        &self.this
    }
}

extern "C" {
    fn napi_get_undefined(env: *const Env, result: *mut *const Value) -> Status;
    fn napi_get_null(env: *const Env, result: *mut *const Value) -> Status;
    fn napi_get_global(env: *const Env, result: *mut *const Value) -> Status;
    fn napi_get_boolean(env: *const Env, value: u8, result: *mut *const Value) -> Status;

    fn napi_create_object(env: *const Env, result: *mut *const Value) -> Status;
    fn napi_create_string_latin1(
        env: *const Env,
        s: *const libc::c_char,
        length: libc::size_t,
        result: *mut *const Value,
    ) -> Status;
    fn napi_create_string_utf8(
        env: *const Env,
        s: *const libc::c_char,
        length: libc::size_t,
        result: *mut *const Value,
    ) -> Status;
    fn napi_create_function(
        env: *const Env,
        utf8name: *const libc::c_char,
        length: libc::size_t,
        cb: Callback,
        data: *mut libc::c_void,
        result: *mut *const Value,
    ) -> Status;

    fn napi_create_double(env: *const Env, value: f64, result: *mut *const Value) -> Status;
    fn napi_create_int32(env: *const Env, value: i32, result: *mut *const Value) -> Status;
    fn napi_create_uint32(env: *const Env, value: u32, result: *mut *const Value) -> Status;
    fn napi_create_int64(env: *const Env, value: i64, result: *mut *const Value) -> Status;

    fn napi_create_array_with_length(
        env: *const Env,
        length: libc::size_t,
        result: *mut *const Value,
    ) -> Status;

    fn napi_create_symbol(
        env: *const Env,
        description: *const Value,
        result: *mut *const Value,
    ) -> Status;

    fn napi_create_error(
        env: *const Env,
        code: *const Value,
        msg: *const Value,
        result: *mut *const Value,
    ) -> Status;
    fn napi_create_type_error(
        env: *const Env,
        code: *const Value,
        msg: *const Value,
        result: *mut *const Value,
    ) -> Status;
    fn napi_create_range_error(
        env: *const Env,
        code: *const Value,
        msg: *const Value,
        result: *mut *const Value,
    ) -> Status;

    fn napi_create_external_arraybuffer(
        env: *const Env,
        external_data: *mut libc::c_void,
        byte_length: libc::size_t,
        finalize_cb: extern "C" fn(*const Env, *mut u8, *mut u8),
        finalize_hint: *mut libc::c_void,
        result: *mut *const Value,
    ) -> Status;

    fn napi_get_value_double(env: *const Env, value: *const Value, result: *mut f64) -> Status;
    fn napi_get_value_int32(env: *const Env, value: *const Value, result: *mut i32) -> Status;
    fn napi_get_value_uint32(env: *const Env, value: *const Value, result: *mut u32) -> Status;
    fn napi_get_value_int64(env: *const Env, value: *const Value, result: *mut i64) -> Status;
    fn napi_get_value_bool(env: *const Env, value: *const Value, result: *mut u8) -> Status;

    fn napi_get_value_string_utf8(
        env: *const Env,
        value: *const Value,
        buf: *mut libc::c_char,
        bufsize: libc::size_t,
        result: *mut libc::size_t,
    ) -> Status;

    fn napi_coerce_to_bool(
        env: *const Env,
        value: *const Value,
        result: *mut *const Value,
    ) -> Status;
    fn napi_coerce_to_number(
        env: *const Env,
        value: *const Value,
        result: *mut *const Value,
    ) -> Status;
    fn napi_coerce_to_object(
        env: *const Env,
        value: *const Value,
        result: *mut *const Value,
    ) -> Status;
    fn napi_coerce_to_string(
        env: *const Env,
        value: *const Value,
        result: *mut *const Value,
    ) -> Status;

    fn napi_set_property(
        env: *const Env,
        object: *const Value,
        key: *const Value,
        value: *const Value,
    ) -> Status;

    fn napi_set_named_property(
        env: *const Env,
        object: *const Value,
        utf8name: *const libc::c_char,
        value: *const Value,
    ) -> Status;

    fn napi_throw(env: *const Env, error: *const Value) -> Status;

    fn napi_throw_error(
        env: *const Env,
        code: *const libc::c_char,
        msg: *const libc::c_char,
    ) -> Status;

    fn napi_throw_range_error(
        env: *const Env,
        code: *const libc::c_char,
        msg: *const libc::c_char,
    ) -> Status;

    fn napi_is_error(env: *const Env, value: *const Value, result: *mut u8) -> Status;

    fn napi_typeof(env: *const Env, value: *const Value, result: *mut ValueType) -> Status;

    fn napi_is_array(env: *const Env, value: *const Value, result: *mut u8) -> Status;
    fn napi_get_array_length(env: *const Env, value: *const Value, result: *mut u32) -> Status;

    fn napi_strict_equals(
        env: *const Env,
        lhs: *const Value,
        rhs: *const Value,
        result: *mut u8,
    ) -> Status;

    fn napi_get_cb_info(
        env: *const Env,
        cbinfo: *const CbInfo,
        argc: *mut libc::size_t,
        argv: *mut *const Value,
        this_arg: *mut *const Value,
        data: *mut *const libc::c_void,
    ) -> Status;
}

/*


NAPI_EXTERN napi_status
napi_get_last_error_info(napi_env env,
                         const napi_extended_error_info** result);

NAPI_EXTERN NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                                 size_t location_len,
                                                 const char* message,
                                                 size_t message_len);

// Getters for defined singletons

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value* result);

NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env,
                                                const char* str,
                                                size_t length,
                                                napi_value* result);
NAPI_EXTERN napi_status napi_create_string_utf16(napi_env env,
                                                 const char16_t* str,
                                                 size_t length,
                                                 napi_value* result);


// Methods to get the the native napi_value from Primitive type

// Copies LATIN-1 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_latin1(napi_env env,
                                                     napi_value value,
                                                     char* buf,
                                                     size_t bufsize,
                                                     size_t* result);

// Copies UTF-8 encoded bytes from a string into a buffer.

// Copies UTF-16 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_utf16(napi_env env,
                                                    napi_value value,
                                                    char16_t* buf,
                                                    size_t bufsize,
                                                    size_t* result);

// Methods to coerce values
// These APIs may execute user scripts

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env,
                                           napi_value object,
                                           napi_value* result);
NAPI_EXTERN napi_status napi_get_property_names(napi_env env,
                                                napi_value object,
                                                napi_value* result);

NAPI_EXTERN napi_status napi_has_property(napi_env env,
                                          napi_value object,
                                          napi_value key,
                                          bool* result);
NAPI_EXTERN napi_status napi_get_property(napi_env env,
                                          napi_value object,
                                          napi_value key,
                                          napi_value* result);
NAPI_EXTERN napi_status napi_delete_property(napi_env env,
                                             napi_value object,
                                             napi_value key,
                                             bool* result);
NAPI_EXTERN napi_status napi_has_own_property(napi_env env,
                                              napi_value object,
                                              napi_value key,
                                              bool* result);

NAPI_EXTERN napi_status napi_has_named_property(napi_env env,
                                          napi_value object,
                                          const char* utf8name,
                                          bool* result);
NAPI_EXTERN napi_status napi_get_named_property(napi_env env,
                                          napi_value object,
                                          const char* utf8name,
                                          napi_value* result);
NAPI_EXTERN napi_status napi_set_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         napi_value value);
NAPI_EXTERN napi_status napi_has_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         bool* result);
NAPI_EXTERN napi_status napi_get_element(napi_env env,
                                         napi_value object,
                                         uint32_t index,
                                         napi_value* result);
NAPI_EXTERN napi_status napi_delete_element(napi_env env,
                                            napi_value object,
                                            uint32_t index,
                                            bool* result);
NAPI_EXTERN napi_status
napi_define_properties(napi_env env,
                       napi_value object,
                       size_t property_count,
                       const napi_property_descriptor* properties);

// Methods to work with Arrays

// Methods to compare values

// Methods to work with Functions
NAPI_EXTERN napi_status napi_call_function(napi_env env,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value* argv,
                                           napi_value* result);
NAPI_EXTERN napi_status napi_new_instance(napi_env env,
                                          napi_value constructor,
                                          size_t argc,
                                          const napi_value* argv,
                                          napi_value* result);
NAPI_EXTERN napi_status napi_instanceof(napi_env env,
                                        napi_value object,
                                        napi_value constructor,
                                        bool* result);

// Methods to work with napi_callbacks

// Gets all callback info in a single call. (Ugly, but faster.)

NAPI_EXTERN napi_status napi_get_new_target(napi_env env,
                                            napi_callback_info cbinfo,
                                            napi_value* result);
NAPI_EXTERN napi_status
napi_define_class(napi_env env,
                  const char* utf8name,
                  size_t length,
                  napi_callback constructor,
                  void* data,
                  size_t property_count,
                  const napi_property_descriptor* properties,
                  napi_value* result);

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap(napi_env env,
                                  napi_value js_object,
                                  void* native_object,
                                  napi_finalize finalize_cb,
                                  void* finalize_hint,
                                  napi_ref* result);
NAPI_EXTERN napi_status napi_unwrap(napi_env env,
                                    napi_value js_object,
                                    void** result);
NAPI_EXTERN napi_status napi_remove_wrap(napi_env env,
                                         napi_value js_object,
                                         void** result);
NAPI_EXTERN napi_status napi_create_external(napi_env env,
                                             void* data,
                                             napi_finalize finalize_cb,
                                             void* finalize_hint,
                                             napi_value* result);
NAPI_EXTERN napi_status napi_get_value_external(napi_env env,
                                                napi_value value,
                                                void** result);

// Methods to control object lifespan

// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
NAPI_EXTERN napi_status napi_create_reference(napi_env env,
                                              napi_value value,
                                              uint32_t initial_refcount,
                                              napi_ref* result);

// Deletes a reference. The referenced value is released, and may
// be GC'd unless there are other references to it.
NAPI_EXTERN napi_status napi_delete_reference(napi_env env, napi_ref ref);

// Increments the reference count, optionally returning the resulting count.
// After this call the  reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
NAPI_EXTERN napi_status napi_reference_ref(napi_env env,
                                           napi_ref ref,
                                           uint32_t* result);

// Decrements the reference count, optionally returning the resulting count.
// If the result is 0 the reference is now weak and the object may be GC'd
// at any time if there are no other references. Calling this when the
// refcount is already 0 results in an error.
NAPI_EXTERN napi_status napi_reference_unref(napi_env env,
                                             napi_ref ref,
                                             uint32_t* result);

// Attempts to get a referenced value. If the reference is weak,
// the value might no longer be available, in that case the call
// is still successful but the result is NULL.
NAPI_EXTERN napi_status napi_get_reference_value(napi_env env,
                                                 napi_ref ref,
                                                 napi_value* result);

NAPI_EXTERN napi_status napi_open_handle_scope(napi_env env,
                                               napi_handle_scope* result);
NAPI_EXTERN napi_status napi_close_handle_scope(napi_env env,
                                                napi_handle_scope scope);
NAPI_EXTERN napi_status
napi_open_escapable_handle_scope(napi_env env,
                                 napi_escapable_handle_scope* result);
NAPI_EXTERN napi_status
napi_close_escapable_handle_scope(napi_env env,
                                  napi_escapable_handle_scope scope);

NAPI_EXTERN napi_status napi_escape_handle(napi_env env,
                                           napi_escapable_handle_scope scope,
                                           napi_value escapee,
                                           napi_value* result);

// Methods to support error handling

// Methods to support catching exceptions
NAPI_EXTERN napi_status napi_is_exception_pending(napi_env env, bool* result);
NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env,
                                                          napi_value* result);

// Methods to provide node::Buffer functionality with napi types
NAPI_EXTERN napi_status napi_create_buffer(napi_env env,
                                           size_t length,
                                           void** data,
                                           napi_value* result);
NAPI_EXTERN napi_status napi_create_external_buffer(napi_env env,
                                                    size_t length,
                                                    void* data,
                                                    napi_finalize finalize_cb,
                                                    void* finalize_hint,
                                                    napi_value* result);
NAPI_EXTERN napi_status napi_create_buffer_copy(napi_env env,
                                                size_t length,
                                                const void* data,
                                                void** result_data,
                                                napi_value* result);
NAPI_EXTERN napi_status napi_is_buffer(napi_env env,
                                       napi_value value,
                                       bool* result);
NAPI_EXTERN napi_status napi_get_buffer_info(napi_env env,
                                             napi_value value,
                                             void** data,
                                             size_t* length);

// Methods to work with array buffers and typed arrays
NAPI_EXTERN napi_status napi_is_arraybuffer(napi_env env,
                                            napi_value value,
                                            bool* result);
NAPI_EXTERN napi_status napi_create_arraybuffer(napi_env env,
                                                size_t byte_length,
                                                void** data,
                                                napi_value* result);

NAPI_EXTERN napi_status napi_get_arraybuffer_info(napi_env env,
                                                  napi_value arraybuffer,
                                                  void** data,
                                                  size_t* byte_length);
NAPI_EXTERN napi_status napi_is_typedarray(napi_env env,
                                           napi_value value,
                                           bool* result);
NAPI_EXTERN napi_status napi_create_typedarray(napi_env env,
                                               napi_typedarray_type type,
                                               size_t length,
                                               napi_value arraybuffer,
                                               size_t byte_offset,
                                               napi_value* result);
NAPI_EXTERN napi_status napi_get_typedarray_info(napi_env env,
                                                 napi_value typedarray,
                                                 napi_typedarray_type* type,
                                                 size_t* length,
                                                 void** data,
                                                 napi_value* arraybuffer,
                                                 size_t* byte_offset);

NAPI_EXTERN napi_status napi_create_dataview(napi_env env,
                                             size_t length,
                                             napi_value arraybuffer,
                                             size_t byte_offset,
                                             napi_value* result);
NAPI_EXTERN napi_status napi_is_dataview(napi_env env,
                                         napi_value value,
                                         bool* result);
NAPI_EXTERN napi_status napi_get_dataview_info(napi_env env,
                                               napi_value dataview,
                                               size_t* bytelength,
                                               void** data,
                                               napi_value* arraybuffer,
                                               size_t* byte_offset);

// Methods to manage simple async operations
NAPI_EXTERN
napi_status napi_create_async_work(napi_env env,
                                   napi_value async_resource,
                                   napi_value async_resource_name,
                                   napi_async_execute_callback execute,
                                   napi_async_complete_callback complete,
                                   void* data,
                                   napi_async_work* result);
NAPI_EXTERN napi_status napi_delete_async_work(napi_env env,
                                               napi_async_work work);
NAPI_EXTERN napi_status napi_queue_async_work(napi_env env,
                                              napi_async_work work);
NAPI_EXTERN napi_status napi_cancel_async_work(napi_env env,
                                               napi_async_work work);

// Methods for custom handling of async operations
NAPI_EXTERN napi_status napi_async_init(napi_env env,
                                        napi_value async_resource,
                                        napi_value async_resource_name,
                                        napi_async_context* result);

NAPI_EXTERN napi_status napi_async_destroy(napi_env env,
                                           napi_async_context async_context);

NAPI_EXTERN napi_status napi_make_callback(napi_env env,
                                           napi_async_context async_context,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value* argv,
                                           napi_value* result);

// version management
NAPI_EXTERN napi_status napi_get_version(napi_env env, uint32_t* result);

NAPI_EXTERN
napi_status napi_get_node_version(napi_env env,
                                  const napi_node_version** version);

// Promises
NAPI_EXTERN napi_status napi_create_promise(napi_env env,
                                            napi_deferred* deferred,
                                            napi_value* promise);
NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env,
                                              napi_deferred deferred,
                                              napi_value resolution);
NAPI_EXTERN napi_status napi_reject_deferred(napi_env env,
                                             napi_deferred deferred,
                                             napi_value rejection);
NAPI_EXTERN napi_status napi_is_promise(napi_env env,
                                        napi_value promise,
                                        bool* is_promise);


*/
