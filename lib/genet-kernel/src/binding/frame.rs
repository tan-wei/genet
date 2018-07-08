use frame::Frame;
use genet_abi::{attr::Attr, layer::Layer, token::Token};
use std::{mem, ptr};

#[no_mangle]
pub extern "C" fn genet_frame_index(frame: *const Frame) -> u32 {
    unsafe { (*frame).index() }
}

#[no_mangle]
pub extern "C" fn genet_frame_layers(frame: *const Frame, len: *mut u32) -> *const *const Layer {
    unsafe {
        let layers = (*frame).layers();
        *len = layers.len() as u32;
        mem::transmute(layers.as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_frame_tree_indices(frame: *const Frame, len: *mut u32) -> *const u8 {
    unsafe {
        let indices = (*frame).tree_indices();
        *len = indices.len() as u32;
        indices.as_ptr()
    }
}

#[no_mangle]
pub extern "C" fn genet_frame_attr(frame: *const Frame, id: Token) -> *const Attr {
    let frame = unsafe { &*frame };
    if let Some(attr) = frame.attr(id) {
        attr
    } else {
        ptr::null()
    }
}
