use crate::{
    decoder::{DecoderData, DecoderStack},
    token::Token,
};
use failure::Fail;
use std::{collections::HashSet, result::Result};

/// A context object.
#[repr(C)]
#[derive(Default)]
pub struct Context {
    decoders: Vec<DecoderData>,
}

impl Context {
    /// Creates a new Context.
    pub fn new() -> Context {
        Self {
            decoders: Vec::new(),
        }
    }

    fn sub_decoders(
        &self,
        used: &mut HashSet<String>,
        id: &str,
        decoders: &[DecoderData],
    ) -> Vec<DecoderStack> {
        let used_decoders = used.clone();
        decoders
            .iter()
            .filter(|d| !used_decoders.contains(&d.id) && d.trigger_after.iter().any(|x| x == id))
            .map(|d| {
                used.insert(d.id.clone());
                DecoderStack::new(
                    d.decoder.new_worker(self),
                    self.sub_decoders(used, &d.id, decoders),
                )
            })
            .collect::<Vec<_>>()
    }

    pub fn decoder<T: Into<Token>>(&self, id: T) -> Result<DecoderStack, ContextError> {
        let id = id.into();
        let strid = id.to_string();
        let mut used = HashSet::new();
        let sub_decoders = self.sub_decoders(&mut used, &strid, &self.decoders);
        self.decoders
            .iter()
            .find(|d| d.id == strid)
            .map(|d| DecoderStack::new(d.decoder.new_worker(self), sub_decoders))
            .ok_or_else(|| ContextError::DecoderNotFound { id })
    }

    pub fn set_decoders(&mut self, decoders: Vec<DecoderData>) {
        self.decoders = decoders
    }
}

#[derive(Debug, Fail)]
pub enum ContextError {
    #[fail(display = "no such decoder: {}", id)]
    DecoderNotFound { id: Token },
}
