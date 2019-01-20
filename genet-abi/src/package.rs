use crate::{
    codable::CodedData,
    decoder::{Decoder, DecoderBox},
    file::FileType,
    reader::{Reader, ReaderBox},
    writer::{Writer, WriterBox},
};
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

impl<T: 'static + Decoder> IntoBuilder<DecoderBuilder> for T {
    fn into_builder(self) -> DecoderBuilder {
        DecoderBuilder {
            data: DecoderData {
                id: String::new(),
                trigger_after: Vec::new(),
                decoder: CodedData::new(DecoderBox::new(self)),
            },
        }
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct DecoderData {
    pub id: String,
    pub trigger_after: Vec<String>,
    pub decoder: CodedData<DecoderBox>,
}

pub struct DecoderBuilder {
    data: DecoderData,
}

impl DecoderBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn trigger_after<T: Into<String>>(mut self, id: T) -> Self {
        self.data.trigger_after.push(id.into());
        self
    }
}

impl Into<DecoderData> for DecoderBuilder {
    fn into(self) -> DecoderData {
        self.data
    }
}

impl Into<Component> for DecoderBuilder {
    fn into(self) -> Component {
        Component::Decoder(self.data)
    }
}

impl<T: 'static + Reader> IntoBuilder<ReaderBuilder> for T {
    fn into_builder(self) -> ReaderBuilder {
        ReaderBuilder {
            data: ReaderData {
                id: String::new(),
                filters: Vec::new(),
                reader: CodedData::new(ReaderBox::new(self)),
            },
        }
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct ReaderData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub reader: CodedData<ReaderBox>,
}

pub struct ReaderBuilder {
    data: ReaderData,
}

impl ReaderBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn filter<T: Into<FileType>>(mut self, file: T) -> Self {
        self.data.filters.push(file.into());
        self
    }
}

impl Into<ReaderData> for ReaderBuilder {
    fn into(self) -> ReaderData {
        self.data
    }
}

impl Into<Component> for ReaderBuilder {
    fn into(self) -> Component {
        Component::Reader(self.data)
    }
}

impl<T: 'static + Writer> IntoBuilder<WriterBuilder> for T {
    fn into_builder(self) -> WriterBuilder {
        WriterBuilder {
            data: WriterData {
                id: String::new(),
                filters: Vec::new(),
                writer: CodedData::new(WriterBox::new(self)),
            },
        }
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub struct WriterData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub writer: CodedData<WriterBox>,
}

pub struct WriterBuilder {
    data: WriterData,
}

impl WriterBuilder {
    pub fn id<T: Into<String>>(mut self, id: T) -> Self {
        self.data.id = id.into();
        self
    }

    pub fn filter<T: Into<FileType>>(mut self, file: T) -> Self {
        self.data.filters.push(file.into());
        self
    }
}

impl Into<WriterData> for WriterBuilder {
    fn into(self) -> WriterData {
        self.data
    }
}

impl Into<Component> for WriterBuilder {
    fn into(self) -> Component {
        Component::Writer(self.data)
    }
}

#[derive(Clone, Deserialize, Serialize)]
pub enum Component {
    Decoder(DecoderData),
    Reader(ReaderData),
    Writer(WriterData),
}
