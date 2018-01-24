use std::io;
use std::io::{Error,ErrorKind};
use std::fs::File;
use std::io::{BufReader,Read};
use std::path::Path;
use plugkit::file::{Importer,RawFrame};

pub struct PcapImporter {
    reader: Option<BufReader<File>>
}

impl PcapImporter {
    pub fn new() -> PcapImporter {
       PcapImporter {
           reader: None,
       }
   }
}

impl<'a> Importer<'a> for PcapImporter {
    fn open(&mut self, path: &Path) -> io::Result<()> {
        if path.ends_with(".pcap") {
            let file = File::open(path)?;
            self.reader = Some(BufReader::new(file));
            Ok(())
        } else {
            Err(Error::new(ErrorKind::Other, "unsupported format"))
        }
    }

    fn run(&mut self) -> io::Result<(&'a[RawFrame], f32)>  {
        let mut buffer = [0; 10];
        let reader = self.reader.as_mut().unwrap();
        reader.read(&mut buffer[..])?;
        Ok((&[], 1.0))
    }
}
