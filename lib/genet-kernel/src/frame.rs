use genet_abi::{layer::Layer, ptr::MutPtr, slice::Slice};
use std::fmt;

pub struct Frame {
    index: u32,
    layers: Vec<MutPtr<Layer>>,
    tree_indices: Vec<u8>,
}

impl fmt::Debug for Frame {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Frame")
    }
}

unsafe impl Send for Frame {}

impl Frame {
    pub fn new(index: u32, root: MutPtr<Layer>) -> Frame {
        Frame {
            index,
            layers: vec![root],
            tree_indices: Vec::new(),
        }
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn layers(&self) -> &[MutPtr<Layer>] {
        &self.layers
    }

    pub fn layers_mut(&mut self) -> &mut Vec<MutPtr<Layer>> {
        &mut self.layers
    }

    pub fn append_layers(&mut self, mut layers: &mut Vec<MutPtr<Layer>>) {
        self.layers.append(&mut layers);
    }

    pub fn tree_indices(&self) -> &[u8] {
        &self.tree_indices
    }

    pub fn append_tree_indices(&mut self, mut tree_indices: &mut Vec<u8>) {
        self.tree_indices.append(tree_indices);
    }
}
