extern crate bincode;
extern crate genet_format;
extern crate genet_sdk;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate serde_derive;

use genet_sdk::{prelude::*, reader::*, variant::Variant};

use std::{
    collections::HashMap,
    fs::File,
    io::{BufReader, Read},
};

#[derive(Deserialize)]
struct Arg {
    file: String,
}

#[derive(Clone)]
struct GenetFileReader {}

fn read_usize(reader: &mut BufReader<File>) -> Result<usize> {
    let mut v = vec![0; bincode::serialized_size(&0usize)? as usize];
    reader.read_exact(&mut v)?;
    let val = bincode::deserialize(&v)?;
    Ok(val)
}

impl Reader for GenetFileReader {
    fn new_worker(&self, _ctx: &Context, arg: &str) -> Result<Box<Worker>> {
        let arg: Arg = serde_json::from_str(arg)?;
        let file = File::open(&arg.file)?;
        let mut reader = BufReader::new(file);

        let mut header_buf = vec![0; read_usize(&mut reader)?];
        reader.read_exact(&mut header_buf)?;
        let header: genet_format::Header = bincode::deserialize(&header_buf)?;
        let tokens: Vec<Token> = header
            .tokens
            .iter()
            .map(|t| Token::from(t.as_str()))
            .collect();

        let attrs = header
            .attrs
            .iter()
            .map(|attr| {
                Fixed::new(attr_class!(tokens[attr.id],
                        typ: tokens[attr.typ]
                    ))
            })
            .collect();

        Ok(Box::new(GenetFileWorker {
            reader,
            header,
            tokens,
            link_layers: HashMap::new(),
            attrs,
        }))
    }

    fn metadata(&self) -> Metadata {
        Metadata {
            id: "app.genet.reader.genet-file".into(),
            filters: vec![FileType::new("genet", &["genet"])],
            ..Metadata::default()
        }
    }
}

struct GenetFileWorker {
    reader: BufReader<File>,
    header: genet_format::Header,
    tokens: Vec<Token>,
    link_layers: HashMap<Token, Fixed<LayerClass>>,
    attrs: Vec<Fixed<AttrClass>>,
}

impl Worker for GenetFileWorker {
    fn read(&mut self) -> Result<Vec<Layer>> {
        let mut layers = Vec::new();
        for _ in 0..self.header.entries {
            let mut frame_buf = vec![0; read_usize(&mut self.reader)?];
            self.reader.read_exact(&mut frame_buf)?;
            let frame: genet_format::Frame = bincode::deserialize(&frame_buf)?;
            let mut payload = vec![0; frame.len];
            self.reader.read_exact(&mut payload)?;

            let id = self.tokens[frame.id];
            let link_class = self
                .link_layers
                .entry(id)
                .or_insert_with(|| Fixed::new(layer_class!(id)));
            let layer = Layer::new(link_class.clone(), ByteSlice::from(payload));
            layers.push(layer);
        }
        self.header.entries = 0;
        Ok(layers)
    }
}

genet_readers!(GenetFileReader {});
