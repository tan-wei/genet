use frame::Frame;
use genet_abi::{attr::Attr, layer::Layer, token::Token};
use std::ptr;

#[no_mangle]
pub unsafe extern "C" fn genet_frame_index(frame: *const Frame) -> u32 {
    (*frame).index()
}

#[no_mangle]
pub unsafe extern "C" fn genet_frame_layers(
    frame: *const Frame,
    len: *mut u32,
) -> *const *const Layer {
    let layers = (*frame).layers();
    *len = layers.len() as u32;
    layers.as_ptr() as *const *const Layer
}

#[no_mangle]
pub unsafe extern "C" fn genet_frame_tree_indices(frame: *const Frame, len: *mut u32) -> *const u8 {
    let indices = (*frame).tree_indices();
    *len = indices.len() as u32;
    indices.as_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn genet_frame_attr(frame: *const Frame, id: Token) -> *const Attr {
    let frame = &*frame;
    if let Some(attr) = frame.attr(id) {
        attr
    } else {
        ptr::null()
    }
}
