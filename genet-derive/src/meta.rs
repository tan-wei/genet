use syn::{Attribute, Lit, Meta, MetaNameValue, NestedMeta};

pub struct AttrMetadata {
    pub id: Option<String>,
    pub typ: Option<String>,
    pub name: Option<String>,
    pub description: Option<String>,
    pub aliases: Vec<String>,
    pub bit_size: Option<usize>,
    pub little_endian: bool,
    pub align_before: bool,
    pub func_map: Option<String>,
    pub func_cond: Option<String>,
    pub skip: bool,
    pub detach: bool,
}

impl AttrMetadata {
    pub fn parse(attrs: &[Attribute]) -> AttrMetadata {
        let mut id = None;
        let mut typ = None;
        let mut aliases = Vec::new();
        let mut docs = String::new();
        let mut bit_size = None;
        let mut skip = false;
        let mut detach = false;
        let mut little_endian = false;
        let mut align_before = false;
        let mut func_map = None;
        let mut func_cond = None;
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
                    ("attr", Meta::List(list)) => {
                        for item in list.nested {
                            if let NestedMeta::Meta(meta) = item {
                                let name = meta.name().to_string();
                                if name == "skip" {
                                    skip = true;
                                } else if name == "detach" {
                                    detach = true;
                                } else if name == "align_before" {
                                    align_before = true;
                                } else if name == "little_endian" {
                                    little_endian = true;
                                } else if name == "big_endian" {
                                    little_endian = false;
                                } else if let Meta::NameValue(MetaNameValue {
                                    lit: Lit::Str(lit_str),
                                    ..
                                }) = meta
                                {
                                    match name.as_str() {
                                        "id" => {
                                            id = Some(lit_str.value().trim().into());
                                        }
                                        "typ" => {
                                            typ = Some(lit_str.value().trim().into());
                                        }
                                        "alias" => {
                                            aliases.push(lit_str.value().to_string());
                                        }
                                        "map" => {
                                            func_map = Some(lit_str.value().to_string());
                                        }
                                        "cond" => {
                                            func_cond = Some(lit_str.value().to_string());
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
        let name = lines
            .next()
            .map(|line| line.trim().to_string())
            .filter(|line| !line.is_empty());
        let description = lines
            .fold(String::new(), |acc, x| acc + x + "\n")
            .trim()
            .to_string();
        let description = if description.is_empty() {
            None
        } else {
            Some(description)
        };

        AttrMetadata {
            id,
            typ,
            name,
            description,
            aliases,
            bit_size,
            skip,
            detach,
            little_endian,
            align_before,
            func_map,
            func_cond,
        }
    }
}
