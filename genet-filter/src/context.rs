use genet_abi::{fixed::MutFixed, layer::Layer};

pub struct Context<'a> {
    layers: &'a [MutFixed<Layer>],
}

impl<'a> Context<'a> {
    pub fn new(layers: &'a [MutFixed<Layer>]) -> Self {
        Context { layers }
    }

    pub fn layers(&self) -> &'a [MutFixed<Layer>] {
        self.layers
    }
}
