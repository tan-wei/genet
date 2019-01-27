use crate::{decoder::DecoderData, reader::ReaderData, writer::WriterData};
use serde_derive::{Deserialize, Serialize};

#[derive(Deserialize, Serialize)]
pub struct Package {
    pub id: String,
    pub name: String,
    pub description: String,
    pub components: Vec<Component>,
}

pub struct PackageBuilder {
    data: Package,
}

impl PackageBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn name<T: Into<String>>(mut self, name: T) -> Self {
        self.data.name = name.into();
        self
    }

    pub fn description<T: Into<String>>(mut self, desc: T) -> Self {
        self.data.description = desc.into();
        self
    }

    pub fn component<T: Into<Component>>(mut self, comp: T) -> Self {
        self.data.components.push(comp.into());
        self
    }
}

impl Package {
    pub fn builder() -> PackageBuilder {
        PackageBuilder {
            data: Package {
                id: String::new(),
                name: String::new(),
                description: String::new(),
                components: Vec::new(),
            },
        }
    }
}

impl Into<Package> for PackageBuilder {
    fn into(self) -> Package {
        self.data
    }
}

pub trait IntoBuilder<T> {
    fn into_builder(self) -> T;
}

#[derive(Clone, Deserialize, Serialize)]
pub enum Component {
    Decoder(DecoderData),
    Reader(ReaderData),
    Writer(WriterData),
}
