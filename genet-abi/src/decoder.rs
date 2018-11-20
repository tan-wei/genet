use bincode;
use context::Context;
use error::Error;
use fixed::MutFixed;
use layer::{Layer, LayerStack, Parent};
use result::Result;
use serde::ser::{Serialize, Serializer};
use std::ptr;
use vec::SafeVec;

/// Execution type.
#[derive(Serialize, Deserialize, Clone, PartialEq, Debug)]
pub enum ExecType {
    ParallelSync,
    SerialSync,
}

/// Decoding status.
#[derive(Debug)]
pub enum Status {
    Done,
    Skip,
}

/// Decoder metadata.
#[derive(Serialize, Deserialize, Debug)]
pub struct Metadata {
    pub id: String,
    pub name: String,
    pub description: String,
    pub exec_type: ExecType,
}

impl Default for Metadata {
    fn default() -> Self {
        Metadata {
            id: String::new(),
            name: String::new(),
            description: String::new(),
            exec_type: ExecType::ParallelSync,
        }
    }
}

/// Decoder worker trait.
pub trait Worker {
    fn decode(&mut self, &mut Context, &LayerStack, &mut Parent) -> Result<Status>;
}

#[repr(C)]
pub struct WorkerBox {
    decode: extern "C" fn(
        *mut WorkerBox,
        *mut Context,
        *const *const Layer,
        u64,
        *mut Parent,
        *mut Error,
    ) -> u8,
    worker: *mut Box<Worker>,
}

impl WorkerBox {
    fn new(worker: Box<Worker>) -> WorkerBox {
        Self {
            decode: abi_decode,
            worker: Box::into_raw(Box::new(worker)),
        }
    }

    pub fn decode(
        &mut self,
        ctx: &mut Context,
        layers: &[MutFixed<Layer>],
        layer: &mut Parent,
    ) -> Result<bool> {
        let stack = layers.as_ptr() as *const *const Layer;
        let mut error = Error::new("");
        let result = (self.decode)(self, ctx, stack, layers.len() as u64, layer, &mut error);
        match result {
            2 => Ok(true),
            1 => Ok(false),
            _ => Err(Box::new(error)),
        }
    }
}

extern "C" fn abi_decode(
    worker: *mut WorkerBox,
    ctx: *mut Context,
    layers: *const *const Layer,
    len: u64,
    layer: *mut Parent,
    error: *mut Error,
) -> u8 {
    let worker = unsafe { &mut *((*worker).worker) };
    let ctx = unsafe { &mut (*ctx) };
    let mut layer = unsafe { &mut *layer };
    let stack = unsafe { LayerStack::new(layers, len as usize) };
    match worker.decode(ctx, &stack, &mut layer) {
        Ok(stat) => match stat {
            Status::Done => 2,
            Status::Skip => 1,
        },
        Err(err) => {
            unsafe {
                ptr::write(error, Error::new(err.description()));
            }
            0
        }
    }
}

/// Decoder trait.
pub trait Decoder: DecoderClone + Send {
    fn new_worker(&self, &Context) -> Box<Worker>;
    fn metadata(&self) -> Metadata;
}

pub trait DecoderClone {
    fn clone_box(&self) -> Box<Decoder>;
    fn into_box(self) -> Box<Decoder>;
}

impl<T> DecoderClone for T
where
    T: 'static + Decoder + Clone,
{
    fn clone_box(&self) -> Box<Decoder> {
        Box::new(self.clone())
    }

    fn into_box(self) -> Box<Decoder> {
        Box::new(self)
    }
}

impl Clone for Box<Decoder> {
    fn clone(&self) -> Box<Decoder> {
        self.clone_box()
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct DecoderBox {
    new_worker: extern "C" fn(*mut DecoderBox, *const Context) -> WorkerBox,
    metadata: extern "C" fn(*const DecoderBox) -> SafeVec<u8>,
    decoder: *mut Box<Decoder>,
}

unsafe impl Send for DecoderBox {}

impl DecoderBox {
    pub fn new<T: 'static + Decoder>(diss: T) -> DecoderBox {
        let diss: Box<Decoder> = Box::new(diss);
        Self {
            new_worker: abi_new_worker,
            metadata: abi_metadata,
            decoder: Box::into_raw(Box::new(diss)),
        }
    }

    pub fn new_worker(&mut self, ctx: &Context) -> WorkerBox {
        (self.new_worker)(self, ctx)
    }

    pub fn metadata(&self) -> Metadata {
        bincode::deserialize(&(self.metadata)(self)).unwrap()
    }
}

impl Serialize for DecoderBox {
    fn serialize<S>(&self, serializer: S) -> std::result::Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        self.metadata().serialize(serializer)
    }
}

extern "C" fn abi_new_worker(diss: *mut DecoderBox, ctx: *const Context) -> WorkerBox {
    let diss = unsafe { &mut *((*diss).decoder) };
    let ctx = unsafe { &(*ctx) };
    WorkerBox::new(diss.new_worker(ctx))
}

extern "C" fn abi_metadata(diss: *const DecoderBox) -> SafeVec<u8> {
    let diss = unsafe { &*((*diss).decoder) };
    bincode::serialize(&diss.metadata()).unwrap().into()
}

#[cfg(test)]
mod tests {
    use attr::AttrClass;
    use context::Context;
    use decoder::{Decoder, DecoderBox, ExecType, Metadata, Status, Worker};
    use fixed::Fixed;
    use fnv::FnvHashMap;
    use layer::{Layer, LayerClass, LayerStack, Parent};
    use result::Result;
    use slice::ByteSlice;
    use token::Token;

    #[test]
    fn decode() {
        struct TestWorker {}

        impl Worker for TestWorker {
            fn decode(
                &mut self,
                _ctx: &mut Context,
                _stack: &LayerStack,
                parent: &mut Parent,
            ) -> Result<Status> {
                let attr = Fixed::new(AttrClass::builder(Token::from(1234)).build());
                let class = Fixed::new(LayerClass::builder(attr).build());
                let layer = Layer::new(class, ByteSlice::new());
                parent.add_child(layer);
                Ok(Status::Done)
            }
        }

        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn new_worker(&self, _ctx: &Context) -> Box<Worker> {
                Box::new(TestWorker {})
            }

            fn metadata(&self) -> Metadata {
                Metadata {
                    exec_type: ExecType::ParallelSync,
                    ..Metadata::default()
                }
            }
        }

        let mut ctx = Context::new(FnvHashMap::default());
        let mut diss = DecoderBox::new(TestDecoder {});
        let mut worker = diss.new_worker(&ctx);

        let attr = Fixed::new(AttrClass::builder(Token::null()).build());
        let class = Fixed::new(LayerClass::builder(attr).build());
        let mut layer = Layer::new(class, ByteSlice::new());
        let mut layer = Parent::from_mut_ref(&mut layer);

        assert_eq!(worker.decode(&mut ctx, &[], &mut layer).unwrap(), true);
    }
}
