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

    let tokens = quote! {};
    tokens.into()
}

fn parse_struct(input: &DeriveInput, s: &DataStruct) -> TokenStream {
    let ident = &input.ident;

    let mut fields_bit_size = Vec::new();
    let mut fields_new = Vec::new();
    let mut fields_class = Vec::new();

    if let Fields::Named(f) = &s.fields {
        for field in &f.named {
            if let Some(ident) = &field.ident {
                let meta = AttrMetadata::parse(&field.attrs);
                let assign_typ = if let Some(typ) = meta.typ {
                    quote! { subctx.typ = #typ.into() }
                } else {
                    quote! {}
                };
                let ty = &field.ty;
                let idstr = normalize_ident(ident);
                fields_bit_size.push(quote! {
                    {
                        type Alias = #ty;
                        let ctx = #ty::context();
                        bit_size += ctx.bit_size;
                    }
                });
                fields_new.push(quote! {
                    #ident: {
                        type Alias = #ty;
                        let mut subctx = #ty::context();
                        #assign_typ;
                        subctx.id = #idstr.into();
                        subctx.path = format!("{}.{}", ctx.path, ctx.id);
                        subctx.bit_offset = bit_offset;
                        bit_offset += subctx.bit_size;
                        Alias::new(&subctx)
                    },
                });
                fields_class.push(quote! {
                    {
                        type Alias = #ty;
                        let mut subctx = #ty::context();
                        #assign_typ;
                        subctx.id = #idstr.into();
                        subctx.path = format!("{}.{}", ctx.path, ctx.id);
                        subctx.bit_offset = bit_offset;
                        bit_offset += subctx.bit_size;
                        Alias::class(&subctx)
                    }
                });
            }
        }
    }

    let tokens = quote! {

        impl genet_sdk::attr::Attr2Field for #ident {
            type Output = genet_sdk::slice::ByteSlice;

            fn context() -> genet_sdk::attr::Attr2Context<Self::Output> {
                let mut bit_size = 0;

                #(
                    #fields_bit_size
                )*

                genet_sdk::attr::Attr2Context {
                    bit_size,
                    ..Default::default()
                }
            }

            fn new(ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> Self {
                let mut bit_offset = ctx.bit_offset;
                Self {
                    #(
                        #fields_new
                    )*
                }
            }

            fn class(ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> genet_sdk::attr::AttrClassBuilder {
                let mut bit_offset = ctx.bit_offset;
                let mut children : Vec<genet_sdk::attr::AttrClassBuilder> = Vec::new();
                #(
                    children.push(#fields_class);
                )*

                genet_sdk::attr::AttrClass::builder(format!("{}.{}", ctx.path, ctx.id).trim_matches('.'))
                    .add_children(children.into_iter().map(|attr| Fixed::new(attr.build())).collect())
                    .bit_range(0, ctx.bit_offset..(ctx.bit_offset + ctx.bit_size))
                    .aliases(ctx.aliases.clone())
                    .name(ctx.name)
                    .description(ctx.description)
                    .typ(&ctx.typ)
            }

            fn build(ctx: &genet_sdk::attr::Attr2Context<Self::Output>) -> genet_sdk::attr::Attr2Functor<Self::Output> {
                genet_sdk::slice::ByteSlice::build(ctx)
            }
        }

    };

    tokens.into()
}
