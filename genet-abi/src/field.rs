use attr::{AttrClass, AttrContext, AttrField, AttrList, SizedAttrField};
use fixed::Fixed;
use std::ops::Deref;

#[derive(Default)]
pub struct Empty();

impl AttrField for Empty {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        AttrList {
            class: Fixed::new(AttrClass::builder("").build()),
            children: Vec::new(),
            attrs: Vec::new(),
            aliases: Vec::new(),
        }
    }
}

pub struct Node<T: SizedAttrField, U: AttrField = Empty> {
    node: T,
    fields: U,
}

impl<T: SizedAttrField, U: AttrField> Node<T, U> {
    pub fn new(node: T, fields: U) -> Self {
        Self { node, fields }
    }

    pub fn fields(&self) -> &U {
        &self.fields
    }
}

impl<T: SizedAttrField, U: AttrField> AttrField for Node<T, U> {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        let mut node = self.node.init(ctx);
        let mut fields = self.fields.init(ctx);
        node.children.append(&mut fields.children);
        node.attrs.append(&mut fields.attrs);
        node.aliases.append(&mut fields.aliases);
        node
    }
}

impl<T: SizedAttrField, U: AttrField> SizedAttrField for Node<T, U> {
    fn bit_size(&self) -> usize {
        self.node.bit_size()
    }
}

impl<T: SizedAttrField + Default, U: AttrField + Default> Default for Node<T, U> {
    fn default() -> Self {
        Self {
            node: T::default(),
            fields: U::default(),
        }
    }
}

impl<T: SizedAttrField, U: AttrField> Deref for Node<T, U> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.node
    }
}
