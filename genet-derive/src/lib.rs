#![recursion_limit = "512"]

extern crate proc_macro;

use inflector::cases::camelcase::to_camel_case;
use proc_macro::TokenStream;
use proc_macro2::Span;
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
        fn class_enum<C: genet_sdk::cast::Typed<Output=T> + 'static + Send + Sync + Clone>(
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
                        .cast(&cast.clone().map(|x| {
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
                .cast(&cast)
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
    let mut fields_ident_mut = Vec::new();
    let mut fields_ident = Vec::new();
    let mut fields_ctx = Vec::new();
    let mut fields_skip = Vec::new();
    let mut fields_detach = Vec::new();
    let mut fields_align = Vec::new();
    let mut fields_filter = Vec::new();
    let mut fields_name = Vec::new();
    let mut fields_name_alias = Vec::new();
    let mut fields_ty = Vec::new();

    if let Fields::Named(f) = &s.fields {
        for field in &f.named {
            if let Some(ident) = &field.ident {
                fields_ty.push(field.ty.clone());
                fields_name_alias.push(Ident::new(
                    &format!("__type__{}", fields_ty.len()),
                    Span::call_site(),
                ));
                fields_name.push(ident);
                let meta = AttrMetadata::parse(&field.attrs);
                let id = if meta.id.is_empty() {
                    normalize_ident(&ident)
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
                let filter = match meta.map {
                    AttrMapExpr::Map(s) => {
                        let expr = syn::parse_str::<Expr>(&s).unwrap();
                        quote! { Some(|x| (#expr).into()) }
                    }
                    AttrMapExpr::Cond(s) => {
                        let expr = syn::parse_str::<Expr>(&s).unwrap();
                        quote! { Some(|x| if (#expr) { x.into() } else { genet_sdk::variant::Variant::Nil }) }
                    }
                    _ => quote! { None },
                };
                fields_filter.push(filter);
                let aliases = meta
                    .aliases
                    .iter()
                    .fold(String::new(), |acc, x| acc + x + " ")
                    .trim()
                    .to_string();
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
                            let attr : &AttrField<I = _> = &self.#ident;
                            (attr, #bit_size)
                        }
                    });
                    fields_ident.push(quote! {
                        {
                            let attr : &AttrField<I = _> = &self.#ident;
                            (attr, #bit_size)
                        }
                    });
                } else {
                    fields_ident_mut.push(quote! {
                        {
                            let sized : &SizedField = &self.#ident;
                            let attr : &AttrField<I = _> = &self.#ident;
                            let size = sized.bit_size();
                            (attr, size)
                        }
                    });
                    fields_ident.push(quote! {
                        {
                            let sized : &SizedField = &self.#ident;
                            let attr : &AttrField<I = _> = &self.#ident;
                            let size = sized.bit_size();
                            (attr, size)
                        }
                    });
                }
            }
        }
    }

    if !fields_align.is_empty() {
        fields_align.drain(..1);
        fields_align.push(false);
    }

    let fields_align2 = fields_align.clone();
    let fields_detach2 = fields_detach.clone();
    let fields_name_alias2 = fields_name_alias.clone();
    let self_attrs = AttrMetadata::parse(&input.attrs);
    let self_name = self_attrs.name;
    let self_desc = self_attrs.description;

    let ident = &input.ident;
    let tokens = quote! {
        impl genet_sdk::attr::AttrField for #ident {
            type I = genet_sdk::slice::ByteSlice;

            fn class(&self, ctx: &::genet_sdk::attr::AttrContext, bit_size: usize, filter: Option<fn(genet_sdk::slice::ByteSlice) -> genet_sdk::variant::Variant>)
                -> genet_sdk::attr::AttrClassBuilder {
                use genet_sdk::attr::{Attr, AttrField, SizedField, AttrContext, AttrClass};
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
                        let child = attr.class(&subctx, bit_size, #fields_filter);

                        if !align && !detach {
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

        impl genet_sdk::attr::SizedField for #ident {
            fn bit_size(&self) -> usize {
                use genet_sdk::attr::{AttrField, SizedField};
                let mut size = 0;

                #(
                    if !#fields_align2 && !#fields_detach2 {
                        let (_, bit_size) = #fields_ident;
                        size += bit_size;
                    }
                )*

                size
            }
        }

        impl genet_sdk::attr::NodeBuilder<Self> for #ident {
            type Builder = Self;

            fn build(ctx: &genet_sdk::context::Context) -> Self {

                #(
                    type #fields_name_alias = #fields_ty;
                )*

                Self {
                    #(
                        #fields_name: #fields_name_alias2 :: build(ctx),
                    )*
                }
            }
        }
    };

    tokens.into()
}
