use genet_abi::{layer::Layer, token::Token};

#[repr(C)]
pub struct Range {
    start: u32,
    end: u32,
}

#[no_mangle]
pub extern "C" fn genet_layer_id(layer: *const Layer) -> Token {
    unsafe { (*layer).id() }
}
