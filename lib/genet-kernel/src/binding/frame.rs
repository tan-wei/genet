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
        let frame = &*frame;
        *len = frame.layers().len() as u32;
        mem::transmute(frame.layers().as_ptr())
    }
}

#[no_mangle]
pub extern "C" fn genet_frame_tree_indices(frame: *const Frame, len: *mut u32) -> *const u8 {
    unsafe {
        let frame = &*frame;
        *len = frame.tree_indices().len() as u32;
        frame.tree_indices().as_ptr()
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
