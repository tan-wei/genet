use crate::decoder::{DecoderData, DecoderStack};
use failure::{format_err, Error};
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
            .filter_map(|d| {
                used.insert(d.id.clone());
                Some(DecoderStack::new(
                    d.decoder.new_worker(self).ok()?,
                    self.sub_decoders(used, &d.id, decoders),
                ))
            })
            .collect::<Vec<_>>()
    }

    pub fn decoder(&self, id: &str) -> Result<DecoderStack, Error> {
        let id = id.to_string();
        let mut used = HashSet::new();
        let sub_decoders = self.sub_decoders(&mut used, &id, &self.decoders);
        self.decoders
            .iter()
            .find(|d| d.id == id)
            .ok_or_else(|| format_err!("no such decoder: {}", id))
            .and_then(|d| Ok(DecoderStack::new(d.decoder.new_worker(self)?, sub_decoders)))
    }

    pub fn set_decoders(&mut self, decoders: Vec<DecoderData>) {
        self.decoders = decoders
    }
}
