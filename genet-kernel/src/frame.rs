use genet_abi::{attr::Attr, fixed::MutFixed, layer::Layer, token::Token};
use std::fmt;

pub struct Frame {
    index: u32,
    root: MutFixed<Layer>,
}

impl fmt::Debug for Frame {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Frame {}", self.index())
    }
}

unsafe impl Send for Frame {}

impl Frame {
    pub fn new(index: u32, root: MutFixed<Layer>) -> Frame {
        Frame { index, root }
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn root(&self) -> &MutFixed<Layer> {
        &self.root
    }

    pub fn layers(&self) -> impl DoubleEndedIterator<Item = &MutFixed<Layer>> {
        vec![self.root()]
            .into_iter()
            .chain(self.root.children_all())
    }

    pub fn attr(&self, id: Token) -> Option<&Attr> {
        for layer in self.layers() {
            if let Some(attr) = layer.attr(id) {
                return Some(attr);
            }
        }
        None
    }
}
