use std::io::{Result,Error,ErrorKind};
use std::path::Path;

pub struct RawFrame {}

pub trait Exporter {
    fn open(_path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run(&[RawFrame]) -> Result<()>  {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}

pub trait Importer<'a> {
    fn open(_path: &Path) -> Result<()> {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }

    fn run() -> Result<(&'a[RawFrame], f32)>  {
        Err(Error::new(ErrorKind::Other, "not implemented"))
    }
}
