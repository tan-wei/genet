use genet_abi::{
    fixed::{Fixed, MutFixed},
    slice::ByteSlice,
};
use libc;
use std::{cell::RefCell, convert::AsRef, ffi::CString, mem, ops::Deref, ptr, rc::Rc};

thread_local! {
    pub static ENV_CONSTRUCTORS: RefCell<Vec<Option<Rc<ValueRef>>>> = RefCell::new(Vec::new());
}

pub type Result<T> = ::std::result::Result<T, Status>;

bitflags! {
    pub struct PropertyAttributes: u32 {
        const DEFAULT = 0;
        const WRITABLE = 1;
        const ENUMERABLE = 1 << 1;
        const CONFIGURABLE = 1 << 2;
        const STATIC = 1 << 10;
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

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum TypedArrayType {
    Int8Array,
    Uint8Array,
    Uint8ClampedArray,
    Int16Array,
    Uint16Array,
    Int32Array,
    Uint32Array,
    Float32Array,
    Float64Array,
    Bigint64Array,
    Biguint64Array,
}

pub enum Env {}

impl Env {
    pub fn get_constructor(&self, index: usize) -> Option<Rc<ValueRef>> {
        ENV_CONSTRUCTORS.with(|v| v.borrow().get(index).cloned().unwrap_or(None))
    }

    pub fn set_constructor(&self, index: usize, value: &Rc<ValueRef>) {
        ENV_CONSTRUCTORS.with(|v| {
            let mut v = v.borrow_mut();
            if v.len() <= index {
                v.resize(index + 1, None)
            }
            v[index] = Some(value.clone());
        })
    }

    pub fn get_undefined<'env>(&self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_undefined(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_null<'env>(&self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_null(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_global<'env>(&self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_global(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_boolean<'env>(&self, value: bool) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_get_boolean(self, if value { 1 } else { 0 }, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_object<'env>(&self) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_object(self, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_string<'env>(&self, s: &str) -> Result<&'env Value> {
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

    fn create_cb<'env>(
        &self,
        func: fn(&'env Env, &CallbackInfo) -> Result<&'env Value>,
    ) -> (Cb, *mut libc::c_void) {
        struct FuncData<'env>(fn(&'env Env, &CallbackInfo) -> Result<&'env Value>);
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
        (cb, unsafe { mem::transmute(FuncData(func)) })
    }

    pub fn create_function<'env>(
        &self,
        name: &str,
        func: fn(&'env Env, &CallbackInfo) -> Result<&'env Value>,
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let (cb, data) = self.create_cb(func);
            match napi_create_function(
                self,
                name.as_ptr() as *const i8,
                name.len(),
                cb,
                data,
                &mut result,
            ) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_double<'env>(&self, value: f64) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_double(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_int32<'env>(&self, value: i32) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_int32(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_uint32<'env>(&self, value: u32) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_uint32(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_int64<'env>(&self, value: i64) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_int64(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_array<'env>(&self, len: usize) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_array_with_length(self, len, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_symbol<'env>(&self, description: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_symbol(self, description, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_error<'env>(&self, code: &Value, msg: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_type_error<'env>(&self, code: &Value, msg: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_type_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_range_error<'env>(&self, code: &Value, msg: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_range_error(self, code, msg, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn create_arraybuffer_copy<'env>(&self, data: &[u8]) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let mut buf: *mut libc::c_void = mem::uninitialized();
            match napi_create_arraybuffer(self, data.len(), &mut buf, &mut result) {
                Status::Ok => {
                    buf.copy_from_nonoverlapping(data.as_ptr() as *const libc::c_void, data.len());
                    Ok(&*result)
                }
                s => Err(s),
            }
        }
    }

    pub fn create_arraybuffer_from_slice<'env>(&self, data: &ByteSlice) -> Result<&'env Value> {
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

    pub fn is_arraybuffer<'env>(&self, value: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_arraybuffer(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_arraybuffer_info<'env>(&self, value: &Value) -> Result<(*const u8, usize)> {
        unsafe {
            let mut data: *const libc::c_void = mem::uninitialized();
            let mut len: usize = mem::uninitialized();
            match napi_get_arraybuffer_info(self, value, &mut data, &mut len) {
                Status::Ok => Ok((data as *const u8, len)),
                s => Err(s),
            }
        }
    }

    pub fn is_typedarray<'env>(&self, value: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_typedarray(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn create_typedarray<'env>(
        &self,
        array_type: TypedArrayType,
        length: usize,
        arraybuffer: &Value,
        offset: usize,
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_create_typedarray(
                self,
                array_type as u32,
                length,
                arraybuffer,
                offset,
                &mut result,
            ) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn get_typedarray_info<'env>(&self, value: &Value) -> Result<(*const u8, usize, usize)> {
        unsafe {
            let mut array_type: u32 = mem::uninitialized();
            let mut data: *const libc::c_void = mem::uninitialized();
            let mut buffer: *const Value = mem::uninitialized();
            let mut len: usize = mem::uninitialized();
            let mut offset: usize = mem::uninitialized();
            match napi_get_typedarray_info(
                self,
                value,
                &mut array_type,
                &mut len,
                &mut data,
                &mut buffer,
                &mut offset,
            ) {
                Status::Ok => Ok((data as *const u8, len, offset)),
                s => Err(s),
            }
        }
    }

    pub fn get_value_double<'env>(&self, value: &Value) -> Result<f64> {
        unsafe {
            let mut result: f64 = mem::uninitialized();
            match napi_get_value_double(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_int32<'env>(&self, value: &Value) -> Result<i32> {
        unsafe {
            let mut result: i32 = mem::uninitialized();
            match napi_get_value_int32(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_uint32<'env>(&self, value: &Value) -> Result<u32> {
        unsafe {
            let mut result: u32 = mem::uninitialized();
            match napi_get_value_uint32(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_int64<'env>(&self, value: &Value) -> Result<i64> {
        unsafe {
            let mut result: i64 = mem::uninitialized();
            match napi_get_value_int64(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn get_value_bool<'env>(&self, value: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_get_value_bool(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_value_string<'env>(&self, value: &Value) -> Result<String> {
        unsafe {
            let mut result: libc::size_t = mem::uninitialized();
            match napi_get_value_string_utf8(self, value, ptr::null_mut(), 0, &mut result) {
                Status::Ok => (),
                s => return Err(s),
            };
            if result == 0 {
                return Ok(String::new());
            }
            let mut v: Vec<u8> = Vec::with_capacity(result + 1);
            v.set_len(result);
            match napi_get_value_string_utf8(
                self,
                value,
                v.as_mut_slice().as_mut_ptr() as *mut i8,
                v.capacity(),
                &mut result,
            ) {
                Status::Ok => Ok(String::from_utf8(v).unwrap()),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_bool<'env>(&self, value: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_bool(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_number<'env>(&self, value: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_number(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_object<'env>(&self, value: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_object(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn coerce_to_string<'env>(&self, value: &Value) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            match napi_coerce_to_string(self, value, &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn set_property<'env>(&self, object: &Value, key: &Value, value: &Value) -> Result<()> {
        unsafe {
            match napi_set_property(self, object, key, value) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn set_element<'env>(&self, object: &Value, index: u32, value: &Value) -> Result<()> {
        unsafe {
            match napi_set_element(self, object, index, value) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn set_named_property<'env>(
        &self,
        object: &Value,
        utf8name: &str,
        value: &Value,
    ) -> Result<()> {
        unsafe {
            let name = CString::new(utf8name).unwrap();
            match napi_set_named_property(self, object, name.as_ptr(), value) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw<'env>(&self, error: &Value) -> Result<()> {
        unsafe {
            match napi_throw(self, error) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw_error<'env>(&self, code: &str, msg: &str) -> Result<()> {
        unsafe {
            let code = CString::new(code).unwrap();
            let msg = CString::new(msg).unwrap();
            match napi_throw_error(self, code.as_ptr(), msg.as_ptr()) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn throw_range_error<'env>(&self, code: &str, msg: &str) -> Result<()> {
        unsafe {
            let code = CString::new(code).unwrap();
            let msg = CString::new(msg).unwrap();
            match napi_throw_range_error(self, code.as_ptr(), msg.as_ptr()) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn is_error<'env>(&self, value: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_error(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn type_of<'env>(&self, value: &Value) -> Result<ValueType> {
        unsafe {
            let mut result: ValueType = mem::uninitialized();
            match napi_typeof(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn is_array<'env>(&self, value: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_is_array(self, value, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_array_length<'env>(&self, value: &Value) -> Result<u32> {
        unsafe {
            let mut result: u32 = mem::uninitialized();
            match napi_get_array_length(self, value, &mut result) {
                Status::Ok => Ok(result),
                s => Err(s),
            }
        }
    }

    pub fn strict_equals<'env>(&self, lhs: &Value, rhs: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_strict_equals(self, lhs, rhs, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn call_function<'env>(
        &self,
        recv: &Value,
        func: &Value,
        argv: &[&Value],
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let argv: Vec<*const Value> = argv.iter().map(|v| *v as *const Value).collect();
            match napi_call_function(self, recv, func, argv.len(), argv.as_ptr(), &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn new_instance<'env>(&self, constructor: &Value, argv: &[&Value]) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let argv: Vec<*const Value> = argv.iter().map(|v| *v as *const Value).collect();
            match napi_new_instance(self, constructor, argv.len(), argv.as_ptr(), &mut result) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn instanceof<'env>(&self, object: &Value, constructor: &Value) -> Result<bool> {
        unsafe {
            let mut result: u8 = mem::uninitialized();
            match napi_instanceof(self, object, constructor, &mut result) {
                Status::Ok => Ok(result != 0),
                s => Err(s),
            }
        }
    }

    pub fn get_cb_info<'env>(&self, cbinfo: &CbInfo) -> Result<CallbackInfo<'env>> {
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

    pub fn define_class<'env>(
        &self,
        name: &str,
        constructor: fn(&'env Env, &CallbackInfo) -> Result<&'env Value>,
        properties: &[PropertyDescriptor],
    ) -> Result<&'env Value> {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            let (cb, data) = self.create_cb(constructor);
            match napi_define_class(
                self,
                name.as_ptr() as *const i8,
                name.len(),
                cb,
                data,
                properties.len(),
                properties.as_ptr(),
                &mut result,
            ) {
                Status::Ok => Ok(&*result),
                s => Err(s),
            }
        }
    }

    pub fn wrap<'env, T>(&self, js_object: &Value, value: T) -> Result<()> {
        extern "C" fn finalize_cb<T>(_env: *const Env, data: *mut u8, _hint: *mut u8) {
            unsafe { Box::from_raw(data as *mut T) };
        }
        let value = Box::into_raw(Box::new(value));
        unsafe {
            match napi_wrap(
                self,
                js_object,
                value as *mut libc::c_void,
                finalize_cb::<T>,
                ptr::null_mut(),
                ptr::null_mut(),
            ) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn wrap_fixed<'env, T>(&self, js_object: &Value, value: &Fixed<T>) -> Result<()> {
        extern "C" fn finalize_cb<T>(_env: *const Env, _data: *mut u8, _hint: *mut u8) {}
        unsafe {
            match napi_wrap(
                self,
                js_object,
                value.as_ptr() as *mut libc::c_void,
                finalize_cb::<T>,
                ptr::null_mut(),
                ptr::null_mut(),
            ) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn wrap_mut_fixed<'env, T>(&self, js_object: &Value, value: &MutFixed<T>) -> Result<()> {
        extern "C" fn finalize_cb<T>(_env: *const Env, _data: *mut u8, _hint: *mut u8) {}
        unsafe {
            match napi_wrap(
                self,
                js_object,
                value.as_ptr() as *mut libc::c_void,
                finalize_cb::<T>,
                ptr::null_mut(),
                ptr::null_mut(),
            ) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn wrap_ptr<'env, T>(&self, js_object: &Value, value: *const T) -> Result<()> {
        extern "C" fn finalize_cb<T>(_env: *const Env, _data: *mut u8, _hint: *mut u8) {}
        unsafe {
            match napi_wrap(
                self,
                js_object,
                value as *mut libc::c_void,
                finalize_cb::<T>,
                ptr::null_mut(),
                ptr::null_mut(),
            ) {
                Status::Ok => Ok(()),
                s => Err(s),
            }
        }
    }

    pub fn unwrap<'env, T>(&self, js_object: &Value) -> Result<&mut T> {
        unsafe {
            let mut result: *mut libc::c_void = mem::uninitialized();
            match napi_unwrap(self, js_object, &mut result) {
                Status::Ok => Ok(&mut *(result as *mut T)),
                s => Err(s),
            }
        }
    }

    pub fn create_ref<'env>(&self, value: &Value) -> Rc<ValueRef> {
        ValueRef::new(self, value)
    }
}

enum Scope {}

pub struct HandleScope {
    env: *const Env,
    scope: *const Scope,
}

impl HandleScope {
    pub fn new(env: &Env) -> HandleScope {
        unsafe {
            let mut result: *const Scope = mem::uninitialized();
            napi_open_handle_scope(env, &mut result);
            HandleScope { env, scope: result }
        }
    }
}

impl Drop for HandleScope {
    fn drop(&mut self) {
        unsafe {
            napi_close_handle_scope(self.env, self.scope);
        }
    }
}

pub enum Value {}
pub enum Ref {}

pub struct ValueRef {
    env: *const Env,
    reference: *const Ref,
}

impl ValueRef {
    fn new(env: *const Env, value: *const Value) -> Rc<ValueRef> {
        unsafe {
            let mut result: *const Ref = mem::uninitialized();
            napi_create_reference(env, value, 1, &mut result);
            Rc::new(ValueRef {
                env,
                reference: result,
            })
        }
    }
}

impl Drop for ValueRef {
    fn drop(&mut self) {
        unsafe {
            let mut result: u32 = mem::uninitialized();
            napi_reference_unref(self.env, self.reference, &mut result);
            napi_delete_reference(self.env, self.reference);
        }
    }
}

impl AsRef<Value> for ValueRef {
    fn as_ref(&self) -> &Value {
        self.deref()
    }
}

impl Deref for ValueRef {
    type Target = Value;

    fn deref(&self) -> &Value {
        unsafe {
            let mut result: *const Value = mem::uninitialized();
            napi_get_reference_value(self.env, self.reference, &mut result);
            &*result
        }
    }
}

pub enum CbInfo {}
type Cb = extern "C" fn(env: *const Env, info: *const CbInfo) -> *const Value;

pub struct CallbackInfo<'env> {
    argv: Vec<&'env Value>,
    this: &'env Value,
    data: *const libc::c_void,
}

impl<'env> CallbackInfo<'env> {
    pub fn argv(&self) -> &[&Value] {
        &self.argv
    }

    pub fn this(&self) -> &Value {
        &self.this
    }
}

#[repr(C)]
pub struct PropertyDescriptor {
    utf8name: *const libc::c_char,
    name: *const Value,
    method: Option<Cb>,
    getter: Option<Cb>,
    setter: Option<Cb>,
    value: *const Value,
    attributes: u32,
    data: *const libc::c_void,
}

impl PropertyDescriptor {
    pub fn new_property<'env>(
        env: &'env Env,
        name: &str,
        attrs: PropertyAttributes,
        accessor: fn(&'env Env, &CallbackInfo) -> Result<&'env Value>,
        setter: bool,
    ) -> PropertyDescriptor {
        let (cb, data) = env.create_cb(accessor);
        PropertyDescriptor {
            utf8name: ptr::null(),
            name: env.create_string(name).unwrap(),
            method: None,
            getter: Some(cb),
            setter: if setter { Some(cb) } else { None },
            value: ptr::null(),
            attributes: attrs.bits(),
            data,
        }
    }

    pub fn new_method<'env>(
        env: &'env Env,
        name: &str,
        attrs: PropertyAttributes,
        method: fn(&'env Env, &CallbackInfo) -> Result<&'env Value>,
    ) -> PropertyDescriptor {
        let (cb, data) = env.create_cb(method);
        PropertyDescriptor {
            utf8name: ptr::null(),
            name: env.create_string(name).unwrap(),
            method: Some(cb),
            getter: None,
            setter: None,
            value: ptr::null(),
            attributes: attrs.bits(),
            data,
        }
    }

    pub fn new_value<'env>(
        env: &'env Env,
        name: &str,
        attrs: PropertyAttributes,
        value: *const Value,
    ) -> PropertyDescriptor {
        PropertyDescriptor {
            utf8name: ptr::null(),
            name: env.create_string(name).unwrap(),
            method: None,
            getter: None,
            setter: None,
            value,
            attributes: attrs.bits(),
            data: ptr::null(),
        }
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
        cb: Cb,
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

    fn napi_is_arraybuffer(env: *const Env, value: *const Value, result: *mut u8) -> Status;

    fn napi_get_arraybuffer_info(
        env: *const Env,
        arraybuffer: *const Value,
        data: *mut *const libc::c_void,
        byte_length: *mut libc::size_t,
    ) -> Status;

    fn napi_create_arraybuffer(
        env: *const Env,
        byte_length: libc::size_t,
        data: *mut *mut libc::c_void,
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

    fn napi_is_typedarray(env: *const Env, value: *const Value, result: *mut u8) -> Status;
    fn napi_create_typedarray(
        env: *const Env,
        array_type: u32,
        length: libc::size_t,
        arraybuffer: *const Value,
        byte_offset: libc::size_t,
        result: *mut *const Value,
    ) -> Status;
    fn napi_get_typedarray_info(
        env: *const Env,
        typedarray: *const Value,
        array_type: *mut u32,
        length: *mut libc::size_t,
        data: *mut *const libc::c_void,
        arraybuffer: *mut *const Value,
        byte_offset: *mut libc::size_t,
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

    fn napi_set_element(
        env: *const Env,
        object: *const Value,
        index: u32,
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

    fn napi_call_function(
        env: *const Env,
        recv: *const Value,
        func: *const Value,
        argc: libc::size_t,
        argv: *const *const Value,
        result: *mut *const Value,
    ) -> Status;

    fn napi_new_instance(
        env: *const Env,
        constructor: *const Value,
        argc: libc::size_t,
        argv: *const *const Value,
        result: *mut *const Value,
    ) -> Status;

    fn napi_instanceof(
        env: *const Env,
        object: *const Value,
        constructor: *const Value,
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

    fn napi_define_class(
        env: *const Env,
        utf8name: *const libc::c_char,
        length: libc::size_t,
        constructor: Cb,
        data: *const libc::c_void,
        property_count: libc::size_t,
        properties: *const PropertyDescriptor,
        result: *mut *const Value,
    ) -> Status;

    fn napi_wrap(
        env: *const Env,
        js_object: *const Value,
        native_object: *mut libc::c_void,
        finalize_cb: extern "C" fn(*const Env, *mut u8, *mut u8),
        finalize_hint: *mut libc::c_void,
        result: *mut *const Ref,
    ) -> Status;

    fn napi_unwrap(
        env: *const Env,
        js_object: *const Value,
        result: *mut *mut libc::c_void,
    ) -> Status;

    fn napi_create_reference(
        env: *const Env,
        value: *const Value,
        initial_refcount: u32,
        result: *mut *const Ref,
    );

    fn napi_delete_reference(env: *const Env, reference: *const Ref) -> Status;

    fn napi_reference_unref(env: *const Env, reference: *const Ref, result: *mut u32) -> Status;

    fn napi_get_reference_value(
        env: *const Env,
        reference: *const Ref,
        result: *mut *const Value,
    ) -> Status;

    fn napi_open_handle_scope(env: *const Env, result: *mut *const Scope) -> Status;
    fn napi_close_handle_scope(env: *const Env, result: *const Scope) -> Status;
}
