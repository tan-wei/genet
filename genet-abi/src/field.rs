use attr::{AttrClass, AttrContext, AttrField, AttrFieldType, AttrList, SizedAttrField};
use fixed::Fixed;
use std::ops::Deref;

pub struct Detach<T: AttrField> {
    attr: T,
    class: Option<Fixed<AttrClass>>,
}

impl<T: AttrField> AttrField for Detach<T> {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        let child = self.attr.init(ctx);
        self.class = Some(child.class.clone());
        child
    }

    fn node_type(&self) -> AttrFieldType {
        AttrFieldType::Detached
    }
}

impl<T: AttrField> SizedAttrField for Detach<T> {
    fn bit_size(&self) -> usize {
        0
    }
}

impl<T: AttrField> Deref for Detach<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.attr
    }
}

impl<T: AttrField + Default> Default for Detach<T> {
    fn default() -> Self {
        Self {
            attr: T::default(),
            class: None,
        }
    }
}

impl<T: AttrField> AsRef<Fixed<AttrClass>> for Detach<T> {
    fn as_ref(&self) -> &Fixed<AttrClass> {
        self.class.as_ref().unwrap()
    }
}

pub struct Node<T: SizedAttrField, U: AttrField> {
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

    fn node_type(&self) -> AttrFieldType {
        AttrFieldType::Attached
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
