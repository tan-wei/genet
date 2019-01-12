#![recursion_limit = "512"]

extern crate proc_macro;

use inflector::cases::camelcase::to_camel_case;
use proc_macro::TokenStream;
use quote::quote;
use std::collections::VecDeque;
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
    let ident = &input.ident;

    let mut fields_class = Vec::new();
    for v in &s.variants {
        let meta = AttrMetadata::parse(&v.attrs);
        let id = if let Some(id) = meta.id {
            id
        } else {
            to_camel_case(&normalize_ident(&v.ident))
        };

        let name = if let Some(name) = meta.name {
            name.into()
        } else {
            to_title_case(&v.ident.to_string())
        };
        let description = meta.description.unwrap_or_else(|| String::new());

        let aliases = meta
            .aliases
            .iter()
            .fold(String::new(), |acc, x| acc + x + " ")
            .trim()
            .to_string();

        let field_ident = &v.ident;

        fields_class.push(quote! {
            {
                let func = T::build(ctx);
                let func_var: Box<Fn(&Attr, &ByteSlice) -> io::Result<genet_sdk::variant::Variant> + Send + Sync> =
                    Box::new(move |attr, data| {
                        (func.func_map)(attr, data)
                            .map(|x| x.into())
                            .map(|x| match x {
                                #ident::#field_ident => genet_sdk::variant::Variant::Bool(true),
                                _ => genet_sdk::variant::Variant::Nil
                            })
                    });

                AttrClass::builder(format!("{}.{}", ctx.path, #id).trim_matches('.'))
                    .cast(func_var)
                    .bit_range(0, ctx.bit_offset..(ctx.bit_offset + ctx.bit_size))
                    .aliases(#aliases
                            .split(' ')
                            .filter(|s| !s.is_empty())
                            .map(|s| s.to_string())
                            .collect())
                    .name(#name)
                    .description(#description)
            }
        });
    }

    let tokens = quote! {
        impl genet_sdk::attr::Enum2Type for #ident {
            type Output = Self;

            fn class<T: genet_sdk::attr::Attr2Field<Output = E>, E: 'static + Into<genet_sdk::variant::Variant> + Into<Self::Output>>(
                ctx: &genet_sdk::attr::Attr2Context<E>,
            ) -> genet_sdk::attr::AttrClassBuilder {
                use std::io;
                use genet_sdk::attr::{AttrClass, Attr2Field};

                let mut children : Vec<genet_sdk::attr::AttrClassBuilder> = Vec::new();

                #(
                    children.push(#fields_class);
                )*

                AttrClass::builder("")
                    .add_children(children.into_iter().map(|attr| Fixed::new(attr.build())).collect())
            }
        }
    };
    tokens.into()
}

fn parse_struct(input: &DeriveInput, s: &DataStruct) -> TokenStream {
    let ident = &input.ident;

    let mut fields_bit_size = Vec::new();
    let mut fields_new = Vec::new();
    let mut fields_eq = Vec::new();
    let mut fields_class = Vec::new();
    let mut fields_align = VecDeque::new();

    if let Fields::Named(f) = &s.fields {
        for field in &f.named {
            if let Some(_) = &field.ident {
                let meta = AttrMetadata::parse(&field.attrs);
                fields_align.push_back(meta.align_before);
            }
        }
    }

    if !fields_align.is_empty() {
        fields_align.pop_front();
        fields_align.push_back(false);
    }

    if let Fields::Named(f) = &s.fields {
        for field in &f.named {
            if let Some(ident) = &field.ident {
                let meta = AttrMetadata::parse(&field.attrs);
                let assign_typ = if let Some(typ) = meta.typ {
                    quote! { subctx.typ = #typ.into() }
                } else {
                    quote! {}
                };
                let assign_bit_size = if let Some(bit_size) = meta.bit_size {
                    quote! { subctx.bit_size = #bit_size }
                } else {
                    quote! {}
                };
                let name = if let Some(name) = meta.name {
                    name.into()
                } else {
                    to_title_case(&ident.to_string())
                };
                let description = meta.description.unwrap_or_else(|| String::new());
                let ty = &field.ty;
                let id = if let Some(id) = meta.id {
                    id
                } else {
                    to_camel_case(&normalize_ident(ident))
                };

                let align_before = fields_align.pop_front() == Some(true);

                if !meta.detach && !align_before {
                    fields_bit_size.push(quote! {
                        {
                            type Alias = #ty;
                            let mut subctx = <Alias as genet_sdk::attr::Attr2Field>::context();
                            #assign_bit_size;
                            bit_size += subctx.bit_size;
                        }
                    });
                }

                let filter = match meta.map {
                    AttrMapExpr::Map(s) => {
                        let expr = syn::parse_str::<Expr>(&s).unwrap();
                        quote! { subctx.func_map = |x| { Ok({ #expr }.into()) }; }
                    }
                    AttrMapExpr::Cond(s) => {
                        let expr = syn::parse_str::<Expr>(&s).unwrap();
                        quote! { subctx.func_cond = |&x| { #expr }; }
                    }
                    _ => quote! {},
                };

                let aliases = meta
                    .aliases
                    .iter()
                    .fold(String::new(), |acc, x| acc + x + " ")
                    .trim()
                    .to_string();

                let assign_bit_offset = if align_before {
                    quote! {}
                } else {
                    quote! { bit_offset += subctx.bit_size }
                };

                let field = quote! {
                    type Alias = #ty;
                    let mut subctx = <Alias as genet_sdk::attr::Attr2Field>::context();
                    #assign_typ;
                    #assign_bit_size;
                    #filter;
                    subctx.id = #id.into();
                    subctx.name = #name;
                    subctx.description = #description;
                    subctx.path = format!("{}.{}", ctx.path, ctx.id);
                    subctx.bit_offset = bit_offset;
                    subctx.aliases = #aliases
                        .split(' ')
                        .filter(|s| !s.is_empty())
                        .map(|s| s.to_string())
                        .collect();
                    #assign_bit_offset;
                };

                fields_new.push(quote! {
                    #ident: {
                        #field
                        <Alias as genet_sdk::attr::Attr2Field>::new(&subctx)
                    },
                });

                // Suppress unused field warnings
                fields_eq.push(quote! {
                    std::ptr::eq(&data.#ident, &data.#ident);
                });

                if !meta.skip && !meta.detach {
                    fields_class.push(quote! {
                        {
                            #field
                            <Alias as genet_sdk::attr::Attr2Field>::class(&subctx)
                        }
                    });
                }
            }
        }
    }

    let tokens = quote! {

        impl genet_sdk::attr::Attr2Field for #ident {
            type Input = genet_sdk::slice::ByteSlice;
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
                let data = Self {
                    #(
                        #fields_new
                    )*
                };
                #(
                    #fields_eq
                )*
                data
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
