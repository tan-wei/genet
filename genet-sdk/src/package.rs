use crate::{decoder::DecoderBox, reader::ReaderBox, writer::WriterBox};
use serde::{
    de::{self, Deserialize, Visitor},
    ser,
};
use serde_derive::{Deserialize, Serialize};
use std::{
    fmt, mem,
    ops::{Deref, DerefMut},
    ptr,
};

pub unsafe trait Codable: Copy {}

unsafe impl Codable for DecoderBox {}
unsafe impl Codable for ReaderBox {}
unsafe impl Codable for WriterBox {}

pub struct CodedData<T: Codable> {
    data: T,
}

impl<T: Codable> Deref for CodedData<T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.data
    }
}

impl<T: Codable> DerefMut for CodedData<T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut self.data
    }
}

impl<T: Codable> CodedData<T> {
    fn new(data: T) -> Self {
        Self { data }
    }

    pub fn unwrap(self) -> T {
        self.data
    }
}

impl<T: Codable> ser::Serialize for CodedData<T> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: ser::Serializer,
        T: Sized,
    {
        let mut buf = vec![0; mem::size_of::<T>()];
        unsafe { ptr::write(buf.as_mut_ptr() as *mut T, self.data) };
        serializer.serialize_bytes(&buf)
    }
}

struct BufVisitor;

impl<'de> Visitor<'de> for BufVisitor {
    type Value = &'de [u8];

    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        formatter.write_str("a byte slice")
    }

    fn visit_borrowed_bytes<E>(self, value: &'de [u8]) -> Result<Self::Value, E>
    where
        E: de::Error,
    {
        Ok(value)
    }
}

impl<'de, T: Codable> Deserialize<'de> for CodedData<T> {
    fn deserialize<D>(deserializer: D) -> Result<CodedData<T>, D::Error>
    where
        D: de::Deserializer<'de>,
    {
        let buf = deserializer.deserialize_bytes(BufVisitor)?;
        unsafe { Ok(CodedData::new(ptr::read(mem::transmute(buf.as_ptr())))) }
    }
}

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

#[derive(Deserialize, Serialize)]
pub struct Decoder {
    id: String,
    trigger_after: Vec<String>,
    decoder: CodedData<DecoderBox>,
}

impl Decoder {
    pub fn builder<T: 'static + crate::decoder::Decoder>(decoder: T) -> DecoderBuilder {
        DecoderBuilder {
            data: Decoder {
                id: String::new(),
                trigger_after: Vec::new(),
                decoder: CodedData::new(DecoderBox::new(decoder)),
            },
        }
    }
}

pub struct DecoderBuilder {
    data: Decoder,
}

impl DecoderBuilder {
    pub fn id<T: Into<String>>(&mut self, id: T) {
        self.data.id = id.into();
    }

    pub fn name<T: Into<String>>(&mut self, id: T) {
        self.data.trigger_after.push(id.into());
    }
}

impl Into<Decoder> for DecoderBuilder {
    fn into(self) -> Decoder {
        self.data
    }
}

impl Into<Component> for DecoderBuilder {
    fn into(self) -> Component {
        Component::Decoder(self.data)
    }
}

#[derive(Deserialize, Serialize)]
pub struct Reader {
    id: String,
    filters: Vec<FileType>,
    reader: CodedData<ReaderBox>,
}

impl Reader {
    pub fn builder<T: 'static + crate::reader::Reader>(reader: T) -> ReaderBuilder {
        ReaderBuilder {
            data: Reader {
                id: String::new(),
                filters: Vec::new(),
                reader: CodedData::new(ReaderBox::new(reader)),
            },
        }
    }
}

pub struct ReaderBuilder {
    data: Reader,
}

impl Into<Component> for ReaderBuilder {
    fn into(self) -> Component {
        Component::Reader(self.data)
    }
}

#[derive(Deserialize, Serialize)]
pub struct Writer {
    id: String,
    filters: Vec<FileType>,
    writer: CodedData<WriterBox>,
}

impl Writer {
    pub fn builder<T: 'static + crate::writer::Writer>(writer: T) -> WriterBuilder {
        WriterBuilder {
            data: Writer {
                id: String::new(),
                filters: Vec::new(),
                writer: CodedData::new(WriterBox::new(writer)),
            },
        }
    }
}

pub struct WriterBuilder {
    data: Writer,
}

impl Into<Component> for WriterBuilder {
    fn into(self) -> Component {
        Component::Writer(self.data)
    }
}

#[derive(Deserialize, Serialize)]
pub enum Component {
    Decoder(Decoder),
    Reader(Reader),
    Writer(Writer),
}

#[derive(Serialize, Deserialize)]
pub struct FileType {
    name: String,
    extensions: Vec<String>,
}
