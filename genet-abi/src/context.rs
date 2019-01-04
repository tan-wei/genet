use crate::{
    decoder::{DecoderBox, DecoderStack},
    token::Token,
};
use std::collections::HashSet;

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

    pub fn sub_decoders(
        &self,
        used: &mut HashSet<String>,
        id: &str,
        decoders: &[DecoderBox],
    ) -> Vec<DecoderStack> {
        let used_decoders = used.clone();
        decoders
            .iter()
            .filter(|d| {
                !used_decoders.contains(&d.metadata().id)
                    && d.metadata().trigger_after.iter().any(|x| x == id)
            })
            .map(|d| {
                used.insert(d.metadata().id);
                DecoderStack::new(
                    d.new_worker(self),
                    self.sub_decoders(used, &d.metadata().id, decoders),
                )
            })
            .collect::<Vec<_>>()
    }

    pub fn decoder<T: Into<Token>>(&self, id: T) -> Option<DecoderStack> {
        let id = id.into().to_string();
        let mut used = HashSet::new();
        let sub_decoders = self.sub_decoders(&mut used, &id, &self.decoders);
        self.decoders
            .iter()
            .find(|d| d.metadata().id == id)
            .map(|d| DecoderStack::new(d.new_worker(self), sub_decoders))
    }

    pub fn set_decoders(&mut self, decoders: Vec<DecoderBox>) {
        self.decoders = decoders
    }
}
