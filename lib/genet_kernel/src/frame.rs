use genet_ffi::layer::Layer;
use genet_ffi::ptr::MutPtr;
use genet_ffi::slice::Slice;
use std::fmt;

pub struct Frame {
    index: u32,
    layers: Vec<MutPtr<Layer>>,
    offsets: Vec<u8>,
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
            offsets: Vec::new(),
        }
    }

    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn layers_mut(&mut self) -> &mut Vec<MutPtr<Layer>> {
        &mut self.layers
    }

    pub fn append_layers(&mut self, mut layers: &mut Vec<MutPtr<Layer>>) {
        self.layers.append(&mut layers);
    }

    pub fn append_offsets(&mut self, mut offsets: &mut Vec<u8>) {
        self.offsets.append(offsets);
    }
}
