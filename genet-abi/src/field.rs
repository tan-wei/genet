use attr::{Attr, AttrClass, AttrContext, AttrField, AttrList, SizedAttrField};
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
    attr: Option<Attr>,
}

impl<T: SizedAttrField, U: AttrField> Node<T, U> {
    pub fn fields(&self) -> &U {
        &self.fields
    }

    pub fn attr(&self) -> &Attr {
        self.attr.as_ref().unwrap()
    }
}

impl<T: SizedAttrField, U: AttrField> AttrField for Node<T, U> {
    fn init(&mut self, ctx: &AttrContext) -> AttrList {
        let mut node = self.node.init(ctx);
        let mut fields = self.fields.init(ctx);
        node.children.append(&mut fields.children);
        node.attrs.append(&mut fields.attrs);
        node.aliases.append(&mut fields.aliases);

        let byte_offset = ctx.bit_offset / 8;
        let bit_offset = ctx.bit_offset - (byte_offset * 8);
        let bit_range = bit_offset..(bit_offset + self.node.bit_size());
        self.attr = Some(
            Attr::builder(node.class.clone())
                .bit_range(byte_offset, bit_range)
                .build(),
        );

        println!("> {:#?}", node.attrs);

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
            attr: None,
        }
    }
}

impl<T: SizedAttrField, U: AttrField> Deref for Node<T, U> {
    type Target = U;

    fn deref(&self) -> &U {
        &self.fields
    }
}
