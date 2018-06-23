use genet_abi::{attr::Attr, token::Token};

#[no_mangle]
pub extern "C" fn genet_attr_id(attr: *const Attr) -> Token {
    unsafe { (*attr).id() }
}

#[no_mangle]
pub extern "C" fn genet_attr_type(attr: *const Attr) -> Token {
    unsafe { (*attr).typ() }
}
