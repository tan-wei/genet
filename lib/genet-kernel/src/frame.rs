use genet_abi::{attr::Attr, fixed::MutFixed, layer::Layer, token::Token};
use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum WorkerMode {
    None,
    Parallel(u8),
    Serial,
}

impl WorkerMode {
    pub fn add(&self, rhs: &WorkerMode) -> WorkerMode {
        match (self, rhs) {
            (WorkerMode::None, rhs) => rhs.clone(),
            (lhs, WorkerMode::None) => lhs.clone(),
            (lhs, rhs) if lhs == rhs => rhs.clone(),
            _ => WorkerMode::Serial,
        }
    }
}

pub struct Frame {
    index: u32,
    layers: Vec<MutFixed<Layer>>,
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
    pub fn new(index: u32, root: MutFixed<Layer>) -> Frame {
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

    pub fn layers(&self) -> &[MutFixed<Layer>] {
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

    pub fn layers_mut(&mut self) -> &mut Vec<MutFixed<Layer>> {
        &mut self.layers
    }

    pub fn append_layers(&mut self, mut layers: &mut Vec<MutFixed<Layer>>) {
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

    pub fn set_worker(&mut self, mode: WorkerMode) {
        self.worker = mode;
    }
}
