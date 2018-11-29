#![recursion_limit = "512"]

#[macro_use]
extern crate quote;

#[macro_use]
extern crate syn;

extern crate genet_abi;
extern crate inflector;
extern crate proc_macro;

use inflector::cases::camelcase::to_camel_case;
use proc_macro::TokenStream;
use syn::{Data, DataStruct, DeriveInput, Fields, Ident};

mod meta;
use meta::AttrMetadata;

mod initialisms;
use initialisms::to_title_case;

#[proc_macro_derive(Attr, attributes(genet))]
pub fn derive_attr(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    match &input.data {
        Data::Struct(s) => parse_struct(&input, &s),
        _ => panic!("Attr derive supports struct types only"),
    }
}

fn normalize_ident(ident: &Ident) -> String {
    let ident = ident.to_string();
    ident.trim_start_matches("r#").into()
}

fn parse_struct(input: &DeriveInput, s: &DataStruct) -> TokenStream {
    let mut fields_ident_mut = Vec::new();
    let mut fields_ident = Vec::new();
    let mut fields_ctx = Vec::new();
    let mut fields_skip = Vec::new();
    let mut fields_detach = Vec::new();
    let mut fields_align = Vec::new();

    if let Fields::Named(f) = &s.fields {
        for field in &f.named {
            if let Some(ident) = &field.ident {
                let meta = AttrMetadata::parse(&field.attrs);
                let id = normalize_ident(&ident);
                let id = format!(".{}", to_camel_case(&id));
                let typ = meta.typ;
                let name = if meta.name.is_empty() {
                    to_title_case(&id)
                } else {
                    meta.name
                };
                let aliases = meta
                    .aliases
                    .iter()
                    .fold(String::new(), |acc, x| acc + x + " ")
                    .trim()
                    .to_string();
                let desc = meta.description;
                fields_ctx.push(quote! {
                    AttrContext{
                        path: format!("{}{}", ctx.path, #id)
                            .trim_matches('.').into(),
                        typ: #typ.into(),
                        name: #name.into(),
                        description: #desc.into(),
                        aliases: #aliases
                            .split(' ')
                            .filter(|s| !s.is_empty())
                            .map(|s| s.to_string())
                            .collect(),
                        ..Default::default()
                    }
                });
                fields_skip.push(meta.skip);
                fields_detach.push(meta.detach);
                fields_align.push(meta.align_before);

                if let Some(bit_size) = meta.bit_size {
                    fields_ident_mut.push(quote! {
                        {
                            let attr : &TypedAttrField<_> = &self.#ident;
                            (attr, #bit_size)
                        }
                    });
                    fields_ident.push(quote! {
                        {
                            let attr : &TypedAttrField<_> = &self.#ident;
                            (attr, #bit_size)
                        }
                    });
                } else {
                    fields_ident_mut.push(quote! {
                        {
                            let sized : &SizedAttrField = &self.#ident;
                            let attr : &TypedAttrField<_> = &self.#ident;
                            let size = sized.bit_size();
                            (attr, size)
                        }
                    });
                    fields_ident.push(quote! {
                        {
                            let sized : &SizedAttrField = &self.#ident;
                            let attr : &TypedAttrField<_> = &self.#ident;
                            let size = sized.bit_size();
                            (attr, size)
                        }
                    });
                }
            }
        }
    }

    fields_align.drain(..1);
    fields_align.push(false);

    let fields_align2 = fields_align.clone();
    let self_attrs = AttrMetadata::parse(&input.attrs);
    let self_name = self_attrs.name;
    let self_desc = self_attrs.description;

    let ident = &input.ident;
    let tokens = quote! {
        impl genet_sdk::attr::TypedAttrField<genet_sdk::slice::ByteSlice> for #ident {
            fn class(&self, ctx: &::genet_sdk::attr::AttrContext, bit_size: usize, filter: fn(&ByteSlice) -> bool)
                -> genet_sdk::attr::AttrClassBuilder {
                use genet_sdk::attr::{Attr, TypedAttrField, SizedAttrField, AttrField, AttrContext, AttrClass};
                use genet_sdk::cast;
                use genet_sdk::fixed::Fixed;

                let mut bit_offset = ctx.bit_offset;
                let mut children = Vec::<Fixed<AttrClass>>::new();

                #(
                    {
                        let mut subctx = #fields_ctx;
                        let (attr, bit_size) = #fields_ident_mut;
                        let skip = #fields_skip;
                        let detach = #fields_detach;
                        let align = #fields_align;

                        subctx.bit_offset = bit_offset;
                        let child = attr.class(&subctx, bit_size, |_| true);

                        if !align {
                            bit_offset += bit_size;
                        }

                        if !skip && !detach {
                            children.push(Fixed::new(child.build()));
                        }
                    }
                )*

                AttrClass::builder(ctx.path.clone())
                    .add_children(children)
                    .aliases(ctx.aliases.clone())
                    .cast(&cast::ByteSlice())
                    .typ(ctx.typ.clone())
                    .bit_range(0, ctx.bit_offset..(ctx.bit_offset + self.bit_size()))
                    .name(if ctx.name.is_empty() {
                        #self_name
                    } else {
                        ctx.name
                    })
                    .description(if ctx.description.is_empty() {
                        #self_desc
                    } else {
                        ctx.description
                    })
            }
        }

        impl genet_sdk::attr::AttrField for #ident {
            fn class(&self, ctx: &::genet_sdk::attr::AttrContext, bit_size: usize)
                -> genet_sdk::attr::AttrClassBuilder {
                genet_sdk::attr::TypedAttrField::<genet_sdk::slice::ByteSlice>::class(self, ctx, bit_size, |_| true)
            }
        }

        impl genet_sdk::attr::SizedAttrField for #ident {
            fn bit_size(&self) -> usize {
                use genet_sdk::attr::{TypedAttrField, SizedAttrField, AttrField};
                let mut size = 0;

                #(
                    if !#fields_align2 {
                        let (_, bit_size) = #fields_ident;
                        size += bit_size;
                    }
                )*

                size
            }
        }
    };

    tokens.into()
}
