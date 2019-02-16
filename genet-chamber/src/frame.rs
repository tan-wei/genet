#![forbid(unsafe_code)]

use crate::{
    attr::{Attr, AttrQuery, AttrTypeSet},
    chamber::Context,
};

pub struct Frame<'a> {
    ctx: &'a Context,
    attrs: &'a [Attr<'a>],
}

impl<'a> Frame<'a> {
    pub fn find(&self, attr: &AttrQuery) -> impl Iterator<Item = Attr<'a>> {
        let id = attr.id();
        let ty = attr.ty();
        self.attrs
            .iter()
            .filter(move |a| {
                a.ty().id_token() == id && (ty.is_none() || ty.unwrap() == a.ty().id_ty())
            })
            .cloned()
    }
}

pub(crate) struct FrameData {
    pub data: Vec<u8>,
    pub attr_sets: Vec<*const AttrTypeSet>,
}
