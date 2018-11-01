use attr::{AttrClass, AttrContext, AttrField, AttrList, SizedAttrField};
use fixed::Fixed;
use num_traits::Num;
use result::Result;
use slice::ByteSlice;
use std::ops::Deref;
use variant::Variant;

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

pub trait Decode: Clone {
    type Output: Into<Variant>;
    fn decode(&self, data: &ByteSlice) -> Result<Self::Output>;
}

impl<V: Into<Variant>, T: Decode<Output = V>> AttrField for T {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        AttrList {
            class: Fixed::new(
                AttrClass::builder(ctx.path.clone())
                    .typ(ctx.typ.clone())
                    .name(ctx.name)
                    .description(ctx.description)
                    .build(),
            ),
            children: Vec::new(),
            attrs: Vec::new(),
            aliases: Vec::new(),
        }
    }
}

impl<V: Into<Variant> + Num, T: Decode<Output = V>> SizedAttrField for T {
    fn bit_size(&self) -> usize {
        std::mem::size_of::<V>()
    }
}
