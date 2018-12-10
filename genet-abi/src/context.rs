use crate::{
    decoder::{DecoderBox, WorkerBox},
    token::Token,
};

/// A context object.
#[repr(C)]
#[derive(Default)]
pub struct Context {
    decoders: Vec<DecoderBox>,
}

impl Context {
    /// Creates a new Context.
    pub fn new() -> Context {
        Self {
            decoders: Vec::new(),
        }
    }

    pub fn decoder<T: Into<Token>>(&self, id: T) -> Option<WorkerBox> {
        let id = id.into().to_string();
        self.decoders
            .iter()
            .find(|d| d.metadata().id == id)
            .map(|d| d.new_worker(self))
    }

    pub fn set_decoders(&mut self, decoders: Vec<DecoderBox>) {
        self.decoders = decoders
    }
}
