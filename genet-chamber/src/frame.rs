#![forbid(unsafe_code)]

use crate::attr::{Attr, AttrQuery, AttrTypeSet};

pub struct Frame<'a> {
    attrs: &'a [Attr<'a>],
}

impl<'a> Frame<'a> {
    pub fn find(&self, attr: &AttrQuery) -> impl Iterator<Item = Attr<'a>> {
        let id = attr.id_token();
        self.attrs
            .iter()
            .filter(move |a| a.ty().id_token() == id)
            .cloned()
    }
}

pub(crate) struct FrameData {
    pub data: Vec<u8>,
    pub attr_sets: Vec<*const AttrTypeSet>,
}
