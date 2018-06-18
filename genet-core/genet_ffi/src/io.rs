use layer::Layer;
use ptr::MutPtr;
use result::Result;

pub trait Writer: Drop {
    fn write(&mut self, layers: &[Layer]) -> Result<()>;
}

pub trait Reader: Drop {
    fn read(&mut self, max: usize) -> Result<Box<[Layer]>>;
}

pub struct WriterBox {
    writer: *mut Box<Writer>,
    drop_writer: extern "C" fn(*mut Box<Writer>),
}

impl WriterBox {
    pub fn new<T: 'static + Writer>(writer: T) -> WriterBox {
        let writer: Box<Writer> = Box::new(writer);
        Self {
            writer: Box::into_raw(Box::new(writer)),
            drop_writer: ffi_drop_writer,
        }
    }

    fn write(&mut self, layers: &[Layer]) -> Result<()> {
        Ok(())
    }
}

impl Drop for WriterBox {
    fn drop(&mut self) {
        (self.drop_writer)(self.writer);
    }
}

pub struct ReaderBox {
    reader: *mut Box<Reader>,
    drop_reader: extern "C" fn(*mut Box<Reader>),
}

impl ReaderBox {
    pub fn new<T: 'static + Reader>(reader: T) -> ReaderBox {
        let reader: Box<Reader> = Box::new(reader);
        Self {
            reader: Box::into_raw(Box::new(reader)),
            drop_reader: ffi_drop_reader,
        }
    }

    pub fn read(&mut self, max: usize) -> Result<Vec<MutPtr<Layer>>> {
        Ok(vec![])
    }
}

impl Drop for ReaderBox {
    fn drop(&mut self) {
        (self.drop_reader)(self.reader);
    }
}

extern "C" fn ffi_drop_writer(writer: *mut Box<Writer>) {
    unsafe { Box::from_raw(writer) };
}

extern "C" fn ffi_drop_reader(reader: *mut Box<Reader>) {
    unsafe { Box::from_raw(reader) };
}
