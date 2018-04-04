extern crate plugkit;
use plugkit::context::SharedContextWrapper;

#[no_mangle]
pub extern "C" fn plugkit_in_create_shared_ctx() -> *mut SharedContextWrapper {
    Box::into_raw(Box::new(SharedContextWrapper::new()))
}

#[no_mangle]
pub extern "C" fn plugkit_in_destroy_shared_ctx(ctx: *mut SharedContextWrapper) {
    unsafe {
        Box::from_raw(ctx);
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn alloc_shared_ctx() {
        super::plugkit_in_destroy_shared_ctx(super::plugkit_in_create_shared_ctx());
    }
}
