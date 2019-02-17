use genet_abi::{attr::Attr, fixed::MutFixed, layer::Layer, token::Token};
use std::{fmt, mem};

pub struct Frame {
    index: u32,
    layers: Vec<MutFixed<Layer>>,
}

impl fmt::Debug for Frame {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Frame {}", self.index())
    }
}

impl Frame {
    pub fn new(index: u32, root: MutFixed<Layer>) -> Frame {
        Frame {
            index,
            layers: vec![root],
        }
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn layers(&self) -> &[MutFixed<Layer>] {
        &self.layers
    }

    pub fn attr(&self, id: Token) -> Option<Attr> {
        for layer in self.layers().iter().rev() {
            if let Some(attr) = layer.attr(id) {
                return Some(attr);
            }
        }
        None
    }

    pub fn fetch_layers(&mut self) -> Vec<MutFixed<Layer>> {
        let mut v = Vec::new();
        mem::swap(&mut self.layers, &mut v);
        v
    }

    pub fn set_layers(&mut self, layers: Vec<MutFixed<Layer>>) {
        self.layers = layers;
    }
}
