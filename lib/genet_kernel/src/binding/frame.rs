use plugkit::{frame::Frame, layer::Layer};

#[no_mangle]
pub extern "C" fn genet_frame_index(frame: *const Frame) -> u32 {
    unsafe { (*frame).index() }
}

#[no_mangle]
pub extern "C" fn genet_frame_root_mut(frame: *mut Frame) -> *mut Layer {
    ::std::ptr::null_mut()
}
