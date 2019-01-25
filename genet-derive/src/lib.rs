#![recursion_limit = "256"]

extern crate proc_macro;

use inflector::cases::camelcase::to_camel_case;
use proc_macro::TokenStream;
use quote::*;
use std::collections::VecDeque;
use syn::{parse_macro_input, Data, DataEnum, DataStruct, DeriveInput, Expr, Fields, Ident};

mod meta;
use crate::meta::{AttrMetadata, ComponentMetadata};

mod initialisms;
use crate::initialisms::to_title_case;

#[proc_macro_derive(Package, attributes(trigger_after, id, file))]
pub fn derive_package(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    let data = if let Data::Struct(s) = &input.data {
        s
    } else {
        unimplemented!();
    };

    let ident = &input.ident;

    let mut components = Vec::new();
    if let Fields::Named(f) = &data.fields {
        for field in &f.named {
            let meta = ComponentMetadata::new(&field.attrs);
            let mut attrs = Vec::new();

            for trigger in meta.trigger_after {
                attrs.push(quote! {
                    .trigger_after(#trigger)
                })
            }

            for file in meta.files {
                let name = &file.name;
                let extensions = file
                    .extensions
                    .iter()
                    .fold(String::new(), |acc, x| acc + x + " ")
                    .trim()
                    .to_string();
                attrs.push(quote! {
                    .filter(genet_sdk::file::FileType {
                        name: #name.into(),
                        extensions: #extensions
                            .split(' ')
                            .filter(|s| !s.is_empty())
                            .map(|s| s.to_string())
                            .collect()
                    })
                })
            }

            let id = &meta.id;
            attrs.push(quote! {
                .id(#id)
            });

            if field.ident.is_some() {
                let ident = &field.ident;
                components.push(quote! {
                    IntoBuilder::into_builder(pkg.#ident)
                        #(
                            #attrs
                        )*
                });
            }
        }
    }

    let tokens = quote! {
        impl Into<genet_sdk::package::Package> for #ident {
            fn into(self) -> genet_sdk::package::Package {
                            use genet_sdk::package::{Package, IntoBuilder};
                            let pkg : #ident = Default::default();
                            Package::builder()
                                .id(env!("CARGO_PKG_NAME"))
                                .name(env!("CARGO_PKG_NAME"))
                                .description(env!("CARGO_PKG_DESCRIPTION"))

                                #(
                                    .component(#components)
                                )*

                                .into()
            }
        }

        #[no_mangle]
        extern "C" fn genet_abi_v1_load_package(data: *mut (), cb: extern "C" fn(*const u8, u64, *mut ())) {
            use genet_sdk::package::Package;
            let pkg : #ident = Default::default().into();
            let pkg : Package = pkg.into();
            let buf = genet_sdk::bincode::serialize(&pkg).unwrap();
            cb(buf.as_ptr(), buf.len() as u64, data);
        }
    };
    tokens.into()
}

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
        let meta = AttrMetadata::new(&v.attrs);
        let id = if let Some(id) = meta.id {
            id
        } else {
            to_camel_case(&normalize_ident(&v.ident))
        };

        let name = if let Some(name) = meta.name {
            name
        } else {
            to_title_case(&v.ident.to_string())
        };
        let description = meta.description.unwrap_or_else(String::new);

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
                let func_map = func.func_map;

                AttrClass::builder(format!("{}.{}", ctx.path, #id).trim_matches('.'))
                    .cast(move |attr, data| {
                    func_map.invoke(attr, data)
                        .map(|x| x.into())
                        .map(|x| match x {
                            #ident::#field_ident => genet_sdk::variant::Variant::Bool(true),
                            _ => genet_sdk::variant::Variant::Nil
                        })
                })
                    .bit_range(ctx.bit_offset..(ctx.bit_offset + ctx.bit_size))
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
        impl genet_sdk::attr::EnumType for #ident {
            type Output = Self;

            fn class<T: 'static + genet_sdk::attr::AttrField<Output = E>, E: 'static + Into<genet_sdk::variant::Variant> + Into<Self::Output>>(
                ctx: &genet_sdk::attr::AttrContext<T::Input, E>,
            ) -> Vec<genet_sdk::attr::AttrClassBuilder> {
                use std::io;
                use genet_sdk::attr::{AttrClass, AttrField};

                let mut children : Vec<genet_sdk::attr::AttrClassBuilder> = Vec::new();

                #(
                    children.push(#fields_class);
                )*

                children
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
            if field.ident.is_some() {
                let meta = AttrMetadata::new(&field.attrs);
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
                let meta = AttrMetadata::new(&field.attrs);
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
                    name
                } else {
                    to_title_case(&ident.to_string())
                };
                let description = meta.description.unwrap_or_else(String::new);
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
                            let mut subctx = <Alias as genet_sdk::attr::AttrField>::context();
                            #assign_bit_size;
                            bit_size += subctx.bit_size;
                        }
                    });
                }

                let func_map = if let Some(func) = meta.func_map {
                    let expr = syn::parse_str::<Expr>(&func).unwrap();
                    quote! { subctx.func_map = |x| { Ok({ #expr }.into()) }; }
                } else {
                    quote! {}
                };

                let func_cond = if let Some(func) = meta.func_cond {
                    let expr = syn::parse_str::<Expr>(&func).unwrap();
                    quote! { subctx.func_cond = |&x| { #expr }; }
                } else {
                    quote! {}
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
                    let mut subctx = <Alias as genet_sdk::attr::AttrField>::context();
                    #assign_typ;
                    #assign_bit_size;
                    #func_map;
                    #func_cond;
                    subctx.id = #id.into();
                    subctx.name = #name;
                    subctx.description = #description;
                    subctx.path = format!("{}.{}", ctx.path, ctx.id).trim_matches('.').into();
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
                        <Alias as genet_sdk::attr::AttrField>::new(&subctx)
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
                            <Alias as genet_sdk::attr::AttrField>::class(&subctx)
                        }
                    });
                }
            }
        }
    }

    let tokens = quote! {

        impl genet_sdk::attr::AttrField for #ident {
            type Input = genet_sdk::slice::ByteSlice;
            type Output = genet_sdk::slice::ByteSlice;

            fn context() -> genet_sdk::attr::AttrContext<Self::Input, Self::Output> {
                let mut bit_size = 0;

                #(
                    #fields_bit_size
                )*

                genet_sdk::attr::AttrContext {
                    bit_size,
                    ..Default::default()
                }
            }

            fn new(ctx: &genet_sdk::attr::AttrContext<Self::Input, Self::Output>) -> Self {
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

            fn class(ctx: &genet_sdk::attr::AttrContext<Self::Input, Self::Output>) -> Vec<genet_sdk::attr::AttrClassBuilder> {
                let mut bit_offset = ctx.bit_offset;
                let mut children : Vec<genet_sdk::attr::AttrClassBuilder> = Vec::new();
                #(
                    children.append(&mut #fields_class);
                )*

                let mut root = vec![genet_sdk::attr::AttrClass::builder(format!("{}.{}", ctx.path, ctx.id).trim_matches('.'))
                    .cast(|_, _| Ok(genet_sdk::variant::Variant::Bool(true)))
                    .bit_range(ctx.bit_offset..(ctx.bit_offset + ctx.bit_size))
                    .aliases(ctx.aliases.clone())
                    .name(ctx.name)
                    .description(ctx.description)
                    .typ(&ctx.typ)];

                root.append(&mut children);
                root
            }

            fn build(ctx: &genet_sdk::attr::AttrContext<Self::Input, Self::Output>) -> genet_sdk::attr::AttrFunctor<Self::Input, Self::Output> {
                genet_sdk::slice::ByteSlice::build(ctx)
            }
        }

    };

    tokens.into()
}
