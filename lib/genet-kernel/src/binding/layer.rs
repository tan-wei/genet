use plugkit::{layer::Layer, token::Token};

#[repr(C)]
pub struct Range {
    start: u32,
    end: u32,
}

#[no_mangle]
pub extern "C" fn genet_layer_new(id: Token) -> *mut Layer {
    ::std::ptr::null_mut()
    //Box::into_raw(Box::new(Layer::from_id(id)))
}

#[no_mangle]
pub extern "C" fn genet_layer_id(layer: *const Layer) -> Token {
    0.into()
}

#[no_mangle]
pub extern "C" fn genet_layer_range(layer: *const Layer) -> Range {
    Range { start: 0, end: 0 }
}

#[no_mangle]
pub extern "C" fn genet_layer_set_range(layer: *mut Layer, range: Range) {}

#[no_mangle]
pub extern "C" fn genet_layer_worker(layer: *const Layer) -> u32 {
    unsafe { (*layer).worker() }
}

#[no_mangle]
pub extern "C" fn genet_layer_set_worker(layer: *mut Layer, worker: u32) {
    unsafe { (*layer).set_worker(worker) }
}

#[no_mangle]
pub extern "C" fn genet_layer_add_child_move(layer: *mut Layer, child: *mut Layer) {
    use std::ptr::read;
    unsafe {
        (*layer).add_child(read(child));
    }
}

#[no_mangle]
pub extern "C" fn genet_layer_free(layer: *mut Layer) {
    unsafe {
        if !layer.is_null() {
            Box::from_raw(layer);
        }
    }
}
