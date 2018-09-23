use genet_abi::{
    attr::Attr,
    layer::{Layer, Payload},
    token::Token,
};
use genet_napi::napi::{
    CallbackInfo, Env, HandleScope, PropertyAttributes, PropertyDescriptor, Result, Status, Value,
    ValueRef,
};
use std::{ptr, rc::Rc};

#[repr(C)]
pub struct Range {
    start: u32,
    end: u32,
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_id(layer: *const Layer) -> Token {
    (*layer).id()
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_attr(layer: *const Layer, id: Token) -> *const Attr {
    if let Some(attr) = (*layer).attr(id) {
        attr
    } else {
        ptr::null()
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    let data = (*layer).data();
    *len = data.len() as u64;
    data.as_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_attrs(
    layer: *const Layer,
    len: *mut u32,
) -> *const *const Attr {
    let attrs = (*layer).attrs();
    *len = attrs.len() as u32;
    attrs.as_ptr() as *const *const Attr
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_headers(
    layer: *const Layer,
    len: *mut u32,
) -> *const *const Attr {
    let headers = (*layer).headers();
    *len = headers.len() as u32;
    headers.as_ptr() as *const *const Attr
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_payloads(
    layer: *const Layer,
    len: *mut u32,
) -> *const Payload {
    let payloads = (*layer).payloads();
    *len = payloads.len() as u32;
    payloads.as_ptr()
}

pub fn wrapper(env: &Env) -> Rc<ValueRef> {
    fn ctor<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        env.get_null()
    }
    let class = env.define_class("Layer", ctor, &vec![]).unwrap();
    env.create_ref(class)
}
