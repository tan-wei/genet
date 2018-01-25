use std::io::{Error, ErrorKind, Result};
use std::path::Path;

pub struct RawFrame {}

pub trait Exporter {
    fn open(&mut self, _path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run(&mut self, &[RawFrame]) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}

pub trait Importer<'a> {
    fn open(&mut self, _path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run(&mut self) -> Result<(&'a [RawFrame], f32)> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}
