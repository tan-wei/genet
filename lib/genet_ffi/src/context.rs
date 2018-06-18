use ptr::MutPtr;

#[repr(C)]
pub struct Context {
    class: &'static ContextClass,
    ffi_unsafe_data: MutPtr<ContextData>,
}

struct ContextData {}

impl Context {
    pub fn new() -> Context {
        Self {
            class: &CONTEXT_CLASS,
            ffi_unsafe_data: MutPtr::new(ContextData {}),
        }
    }
}

#[repr(u64)]
#[derive(PartialEq, PartialOrd)]
#[allow(dead_code)]
enum Revision {
    Id = 0,
}

#[repr(C)]
pub struct ContextClass {
    rev: Revision,
}

impl ContextClass {
    fn new() -> ContextClass {
        Self { rev: Revision::Id }
    }
}

lazy_static! {
    static ref CONTEXT_CLASS: ContextClass = ContextClass::new();
}
