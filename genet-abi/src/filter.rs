use fixed::MutFixed;
use layer::Layer;

pub struct LayerContext<'a> {
    layers: &'a [MutFixed<Layer>],
}

impl<'a> LayerContext<'a> {
    pub fn new(layers: &'a [MutFixed<Layer>]) -> Self {
        LayerContext { layers }
    }

    pub fn layers(&self) -> &'a [MutFixed<Layer>] {
        self.layers
    }
}

pub trait LayerFilter {
    fn test(&self, ctx: &LayerContext) -> bool;
}
