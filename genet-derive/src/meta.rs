use syn::{Attribute, Lit, Meta, MetaNameValue, NestedMeta};

pub enum AttrMapExpr {
    None,
    Map(String),
    Cond(String),
    CondEq(String),
}

pub struct AttrMetadata {
    pub typ: String,
    pub name: String,
    pub description: String,
    pub aliases: Vec<String>,
    pub bit_size: Option<usize>,
    pub align_before: bool,
    pub map: AttrMapExpr,
    pub skip: bool,
    pub detach: bool,
}

impl AttrMetadata {
    pub fn parse(attrs: &[Attribute]) -> AttrMetadata {
        let mut typ = String::new();
        let mut aliases = Vec::new();
        let mut docs = String::new();
        let mut bit_size = None;
        let mut skip = false;
        let mut detach = false;
        let mut align_before = false;
        let mut map = AttrMapExpr::None;
        for attr in attrs {
            if let Some(meta) = attr.interpret_meta() {
                let name = meta.name().to_string();
                match (name.as_str(), meta) {
                    (
                        "doc",
                        Meta::NameValue(MetaNameValue {
                            lit: Lit::Str(lit_str),
                            ..
                        }),
                    ) => {
                        docs += &lit_str.value();
                        docs += "\n";
                    }
                    ("genet", Meta::List(list)) => {
                        for item in list.nested {
                            if let NestedMeta::Meta(meta) = item {
                                let name = meta.name().to_string();
                                if name == "skip" {
                                    skip = true;
                                } else if name == "detach" {
                                    detach = true;
                                } else if name == "align_before" {
                                    align_before = true;
                                } else if let Meta::NameValue(MetaNameValue {
                                    lit: Lit::Str(lit_str),
                                    ..
                                }) = meta
                                {
                                    match name.as_str() {
                                        "typ" => {
                                            typ = lit_str.value().to_string();
                                        }
                                        "alias" => {
                                            aliases.push(lit_str.value().to_string());
                                        }
                                        "map" => {
                                            map = AttrMapExpr::Map(lit_str.value().to_string());
                                        }
                                        "cond" => {
                                            map = AttrMapExpr::Cond(lit_str.value().to_string());
                                        }
                                        "cond_eq" => {
                                            map = AttrMapExpr::CondEq(lit_str.value().to_string());
                                        }
                                        _ => panic!("unsupported attribute: {}", name),
                                    }
                                } else if let Meta::NameValue(MetaNameValue {
                                    lit: Lit::Int(lit_int),
                                    ..
                                }) = meta
                                {
                                    match name.as_str() {
                                        "byte_size" => {
                                            bit_size = Some(lit_int.value() as usize * 8);
                                        }
                                        "bit_size" => {
                                            bit_size = Some(lit_int.value() as usize);
                                        }
                                        _ => panic!("unsupported attribute: {}", name),
                                    }
                                }
                            }
                        }
                    }
                    _ => {}
                }
            }
        }
        let mut lines = docs.split('\n').map(|line| line.trim());
        let name = lines.next().unwrap_or("");
        let description = lines.fold(String::new(), |acc, x| acc + x + "\n");

        AttrMetadata {
            typ: typ.trim().into(),
            name: name.into(),
            description: description.trim().into(),
            aliases,
            bit_size,
            skip,
            detach,
            align_before,
            map,
        }
    }
}
