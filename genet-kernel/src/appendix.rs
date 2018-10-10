use evmap::ShallowCopy;
use genet_abi::{layer::Payload, attr::Attr, fixed::Fixed};

#[derive(Clone)]
struct LayerData {
    attrs: Vec<Fixed<Attr>>,
    payloads: Vec<Payload>,
}

#[derive(Clone)]
pub enum Content {
    Layer(LayerData)
}

impl ShallowCopy for Content {
    unsafe fn shallow_copy(&mut self) -> Self {
        self.clone()
    }
}
