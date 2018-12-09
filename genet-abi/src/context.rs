use crate::fixed::Fixed;

/// A context object.
#[repr(C)]
pub struct Context {
    parent: Option<Fixed<Context>>,
}

impl Context {
    /// Creates a new Context.
    pub fn new(parent: Option<Fixed<Context>>) -> Context {
        Self { parent }
    }
}

impl Default for Context {
    fn default() -> Self {
        Self::new(None)
    }
}
