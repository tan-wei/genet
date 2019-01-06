#![recursion_limit = "512"]

extern crate proc_macro;

use inflector::cases::camelcase::to_camel_case;
use proc_macro::TokenStream;
use quote::quote;
use syn::{parse_macro_input, Data, DataEnum, DataStruct, DeriveInput, Expr, Fields, Ident};

mod meta;
use crate::meta::{AttrMapExpr, AttrMetadata};

mod initialisms;
use crate::initialisms::to_title_case;

#[proc_macro_derive(Attr, attributes(genet))]
pub fn derive_attr(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    match &input.data {
        Data::Struct(s) => parse_struct(&input, &s),
        Data::Enum(e) => parse_enum(&input, &e),
        _ => panic!("Attr derive supports struct and enum types only"),
    }
}

fn normalize_ident(ident: &Ident) -> String {
    let ident = ident.to_string();
    ident.trim_start_matches("r#").into()
}

fn parse_enum(input: &DeriveInput, s: &DataEnum) -> TokenStream {
    let mut fields_ctx = Vec::new();
    let mut fields_variant = Vec::new();
    let mut fields_ident = Vec::new();

    let ident = &input.ident;
    for v in &s.variants {
        let meta = AttrMetadata::parse(&v.attrs);
        let id = if meta.id.is_empty() {
            normalize_ident(&v.ident)
        } else {
            meta.id
        };
        let id = format!(".{}", to_camel_case(&id));

        let typ = meta.typ;
        let name = if meta.name.is_empty() {
            to_title_case(&id)
        } else {
            meta.name
        };
        let desc = meta.description;

        fields_ctx.push(quote! {
            AttrContext{
                path: format!("{}{}", ctx.path, #id)
                    .trim_matches('.').into(),
                typ: #typ.into(),
                name: #name.into(),
                description: #desc.into(),
                ..Default::default()
            }
        });
        fields_variant.push(v.ident.clone());
        fields_ident.push(ident.clone());
    }

    let fields_ident2 = fields_ident.clone();
    let self_attrs = AttrMetadata::parse(&input.attrs);
    let self_name = self_attrs.name;
    let self_desc = self_attrs.description;

    let tokens = quote! {

    impl<T: Into<genet_sdk::variant::Variant> + Into<#ident> + 'static + Clone> genet_sdk::attr::EnumField<T> for #ident {
        fn class_enum<C: genet_sdk::cast::Typed<Output=T> + Clone>(
            &self,
            ctx: &genet_sdk::attr::AttrContext,
            bit_size: usize,
            cast: C
        ) -> genet_sdk::attr::AttrClassBuilder {
            use genet_sdk::attr::{AttrClass, AttrContext};

            let bit_range = ctx.bit_offset..(ctx.bit_offset + bit_size);
            let mut children = Vec::<Fixed<AttrClass>>::new();

            #(
                {
                    use genet_sdk::variant::Variant;
                    let mut subctx = #fields_ctx;
                    subctx.bit_offset = ctx.bit_offset;
                    let child = AttrClass::builder(subctx.path.clone())
                        .cast(cast.clone().map(|x| {
                            let x : #fields_ident = x.into();
                            match x {
                                #fields_ident2::#fields_variant => Variant::Bool(true),
                                _ => Variant::Nil,
                            }
                        }))
                        .typ(if subctx.typ.is_empty() {
                            "@novalue".into()
                        } else {
                            subctx.typ.clone()
                        })
                        .bit_range(0, bit_range.clone())
                        .name(subctx.name)
                        .description(subctx.description);
                    children.push(Fixed::new(child.build()));
                }
            )*

            AttrClass::builder(ctx.path.clone())
                .add_children(children)
                .cast(cast)
                .typ(if ctx.typ.is_empty() {
                    "@enum".into()
                } else {
                    ctx.typ.clone()
                })
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
                .bit_range(0, bit_range)
        }
    }

    };
    tokens.into()
}

fn parse_struct(input: &DeriveInput, s: &DataStruct) -> TokenStream {
    let ident = &input.ident;

    let tokens = quote! {

        impl genet_sdk::attr::Attr2Field for #ident {
            type Output = genet_sdk::slice::ByteSlice;

            fn context() -> genet_sdk::attr::Attr2Context<Self::Output> {
                genet_sdk::attr::Attr2Context {
                    ..Default::default()
                }
            }

            fn new(_ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> Self {
                Self::default()
            }

            fn class(ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> genet_sdk::attr::AttrClassBuilder {
                genet_sdk::attr::AttrClass::builder("")
            }

            fn build(ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> genet_sdk::attr::Attr2Functor<Self::Output> {
                genet_sdk::slice::ByteSlice::build(ctx)
            }
        }

    };

    tokens.into()
}
