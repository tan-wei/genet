use crate::{
    codable::CodedData, decoder::DecoderBox, file::FileType, reader::ReaderBox, writer::WriterBox,
};
use serde_derive::{Deserialize, Serialize};

#[derive(Deserialize, Serialize)]
pub struct Package {
    id: String,
    name: String,
    description: String,
    components: Vec<Component>,
}

pub struct PackageBuilder {
    data: Package,
}

impl PackageBuilder {
    pub fn id<T: Into<String>>(&mut self, id: T) {
        self.data.id = id.into();
    }

    pub fn name<T: Into<String>>(&mut self, name: T) {
        self.data.name = name.into();
    }

    pub fn description<T: Into<String>>(&mut self, desc: T) {
        self.data.description = desc.into();
    }

    pub fn component<T: Into<Component>>(&mut self, comp: T) {
        self.data.components.push(comp.into());
    }
}

impl Package {
    pub fn builder() -> PackageBuilder {
        PackageBuilder {
            data: Package {
                id: String::new(),
                name: env!("CARGO_PKG_NAME").to_string(),
                description: env!("CARGO_PKG_DESCRIPTION").to_string(),
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

#[derive(Clone, Deserialize, Serialize)]
pub struct DecoderData {
    pub id: String,
    pub trigger_after: Vec<String>,
    pub decoder: CodedData<DecoderBox>,
}

impl DecoderData {
    pub fn builder(decoder: DecoderBox) -> DecoderBuilder {
        DecoderBuilder {
            data: DecoderData {
                id: String::new(),
                trigger_after: Vec::new(),
                decoder: CodedData::new(decoder),
            },
        }
    }
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

#[derive(Clone, Deserialize, Serialize)]
pub struct ReaderData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub reader: CodedData<ReaderBox>,
}

impl ReaderData {
    pub fn builder(reader: ReaderBox) -> ReaderBuilder {
        ReaderBuilder {
            data: ReaderData {
                id: String::new(),
                filters: Vec::new(),
                reader: CodedData::new(reader),
            },
        }
    }
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

#[derive(Clone, Deserialize, Serialize)]
pub struct WriterData {
    pub id: String,
    pub filters: Vec<FileType>,
    pub writer: CodedData<WriterBox>,
}

impl WriterData {
    pub fn builder(writer: WriterBox) -> WriterBuilder {
        WriterBuilder {
            data: WriterData {
                id: String::new(),
                filters: Vec::new(),
                writer: CodedData::new(writer),
            },
        }
    }
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
