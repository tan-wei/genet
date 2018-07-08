use genet_abi::{
    attr::Attr,
    layer::{Layer, Payload},
    token::Token,
};
use std::{mem, ptr};

#[repr(C)]
pub struct Range {
    start: u32,
    end: u32,
}

#[no_mangle]
pub extern "C" fn genet_layer_id(layer: *const Layer) -> Token {
    unsafe { (*layer).id() }
}

#[no_mangle]
pub extern "C" fn genet_layer_attr(layer: *const Layer, id: Token) -> *const Attr {
    unsafe {
        if let Some(attr) = (*layer).attr(id) {
            attr
        } else {
            ptr::null()
        }
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    unsafe {
        let data = (*layer).data();
        *len = data.len() as u64;
        mem::transmute(data.as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_attrs(layer: *const Layer, len: *mut u32) -> *const *const Attr {
    unsafe {
        let attrs = (*layer).attrs();
        *len = attrs.len() as u32;
        mem::transmute(attrs.as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_headers(layer: *const Layer, len: *mut u32) -> *const *const Attr {
    unsafe {
        let headers = (*layer).headers();
        *len = headers.len() as u32;
        mem::transmute(headers.as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_payloads(layer: *const Layer, len: *mut u32) -> *const Payload {
    unsafe {
        let payloads = (*layer).payloads();
        *len = payloads.len() as u32;
        payloads.as_ptr()
    }
}
