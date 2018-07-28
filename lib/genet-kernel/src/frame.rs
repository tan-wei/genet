use genet_abi::{attr::Attr, layer::Layer, ptr::MutPtr, token::Token};
use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum WorkerMode {
    None,
    Parallel(u8),
    Serial,
}

pub struct Frame {
    index: u32,
    layers: Vec<MutPtr<Layer>>,
    tree_indices: Vec<u8>,
    worker: WorkerMode,
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
            worker: WorkerMode::None,
        }
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn layers(&self) -> &[MutPtr<Layer>] {
        &self.layers
    }

    pub fn attr(&self, id: Token) -> Option<&Attr> {
        for layer in self.layers().iter().rev() {
            if let Some(attr) = layer.attr(id) {
                return Some(attr);
            }
        }
        None
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

    pub fn append_tree_indices(&mut self, tree_indices: &mut Vec<u8>) {
        self.tree_indices.append(tree_indices);
    }

    pub fn worker(&self) -> &WorkerMode {
        &self.worker
    }

    pub fn set_worker(&mut self, id: u8) {
        self.worker = match self.worker {
            WorkerMode::None => WorkerMode::Parallel(id),
            WorkerMode::Parallel(w) if w == id => WorkerMode::Parallel(id),
            _ => WorkerMode::Serial,
        };
    }
}
