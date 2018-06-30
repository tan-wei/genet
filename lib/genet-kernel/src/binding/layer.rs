use genet_abi::{attr::Attr, layer::Layer, token::Token};
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
        let layer = &*layer;
        *len = layer.data().len() as u64;
        mem::transmute(layer.data().as_ptr())
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
