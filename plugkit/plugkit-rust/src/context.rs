use super::variant::Variant;
use super::symbol;

pub enum Context {}

impl Context {
    pub fn get_option(&self, name: &str) -> &Variant {
        unsafe {
            &*symbol::Context_getOption.unwrap()(self, name.as_ptr() as *const i8)
        }
    }
}
