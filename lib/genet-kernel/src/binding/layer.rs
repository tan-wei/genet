use genet_abi::{attr::Attr, layer::Layer, token::Token};
use std::mem;
use std::ptr;

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
pub extern "C" fn genet_layer_attrs(layer: *const Layer, len: *mut u32) -> *const *const Attr {
    unsafe {
        let layer = &*layer;
        *len = layer.attrs().len() as u32;
        mem::transmute(layer.attrs().as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_headers(layer: *const Layer, len: *mut u32) -> *const *const Attr {
    unsafe {
        let layer = &*layer;
        *len = layer.headers().len() as u32;
        mem::transmute(layer.headers().as_ptr())
    }
}
