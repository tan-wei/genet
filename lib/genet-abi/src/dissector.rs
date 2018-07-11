use context::Context;
use error::Error;
use layer::Layer;
use ptr::MutPtr;
use result::Result;
use std::{mem, ptr, str};
use string::SafeString;

pub enum Status {
    Done(Vec<Layer>),
    Skip,
}

pub trait Worker {
    fn analyze(&mut self, &mut Context, &mut Layer) -> Result<Status>;
}

#[repr(C)]
pub struct WorkerBox {
    analyze:
        extern "C" fn(*mut WorkerBox, *mut Context, *mut Layer, *mut MutPtr<Layer>, *mut Error)
            -> u8,
    worker: *mut Box<Worker>,
}

const MAX_CHILDREN: usize = <u8>::max_value() as usize - 2;

impl WorkerBox {
    fn new(worker: Box<Worker>) -> WorkerBox {
        Self {
            analyze: abi_analyze,
            worker: Box::into_raw(Box::new(worker)),
        }
    }

    pub fn analyze(
        &mut self,
        ctx: &mut Context,
        layer: &mut Layer,
        results: &mut Vec<MutPtr<Layer>>,
    ) -> Result<bool> {
        let mut children: [MutPtr<Layer>; MAX_CHILDREN];
        unsafe {
            children = mem::uninitialized();
        }
        let mut error = Error::new("");
        let result = (self.analyze)(self, ctx, layer, children.as_mut_ptr(), &mut error);
        if result > 1 {
            let len = result as usize - 2;
            unsafe {
                let grow = len.saturating_sub(results.capacity());
                results.reserve(grow);
                results.set_len(len);
                ptr::copy(children.as_ptr(), results.as_mut_ptr(), len);
            }
            Ok(true)
        } else if result > 0 {
            Ok(false)
        } else {
            Err(Box::new(error))
        }
    }
}

extern "C" fn abi_analyze(
    worker: *mut WorkerBox,
    ctx: *mut Context,
    layer: *mut Layer,
    children: *mut MutPtr<Layer>,
    error: *mut Error,
) -> u8 {
    let worker = unsafe { &mut *((*worker).worker) };
    let ctx = unsafe { &mut (*ctx) };
    let layer = unsafe { &mut (*layer) };
    match worker.analyze(ctx, layer) {
        Ok(stat) => match stat {
            Status::Done(layers) => {
                let len = 2u8.saturating_add(layers.len() as u8);
                for (i, layer) in layers.into_iter().take(MAX_CHILDREN).enumerate() {
                    unsafe { ptr::write(children.offset(i as isize), MutPtr::new(layer)) };
                }
                len
            }
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

pub trait Dissector: DissectorClone + Send {
    fn new_worker(&self, &str, &Context) -> Option<Box<Worker>>;
}

pub trait DissectorClone {
    fn clone_box(&self) -> Box<Dissector>;
}

impl<T> DissectorClone for T
where
    T: 'static + Dissector + Clone,
{
    fn clone_box(&self) -> Box<Dissector> {
        Box::new(self.clone())
    }
}

impl Clone for Box<Dissector> {
    fn clone(&self) -> Box<Dissector> {
        self.clone_box()
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct DissectorBox {
    new_worker: extern "C" fn(*mut DissectorBox, SafeString, *const Context, *mut WorkerBox) -> u8,
    dissector: *mut Box<Dissector>,
}

unsafe impl Send for DissectorBox {}

impl DissectorBox {
    pub fn new<T: 'static + Dissector>(diss: T) -> DissectorBox {
        let diss: Box<Dissector> = Box::new(diss);
        Self {
            new_worker: abi_new_worker,
            dissector: Box::into_raw(Box::new(diss)),
        }
    }

    pub fn new_worker(&mut self, typ: &str, ctx: &Context) -> Option<WorkerBox> {
        let typ = SafeString::from(typ);
        let mut worker;
        unsafe {
            worker = mem::uninitialized();
        }
        if (self.new_worker)(self, typ, ctx, &mut worker) == 1 {
            Some(worker)
        } else {
            mem::forget(worker);
            None
        }
    }
}

extern "C" fn abi_new_worker(
    diss: *mut DissectorBox,
    typ: SafeString,
    ctx: *const Context,
    dst: *mut WorkerBox,
) -> u8 {
    let diss = unsafe { &mut *((*diss).dissector) };
    let ctx = unsafe { &(*ctx) };
    if let Some(worker) = diss.new_worker(typ.as_str(), ctx) {
        unsafe { ptr::write(dst, WorkerBox::new(worker)) };
        1
    } else {
        0
    }
}

#[cfg(test)]
mod tests {
    use context::Context;
    use dissector::{Dissector, DissectorBox, Status, Worker};
    use layer::{Layer, LayerBuilder};
    use result::Result;
    use slice::Slice;
    use std::collections::HashMap;
    use token::Token;

    #[test]
    fn analyze() {
        struct TestWorker {}

        impl Worker for TestWorker {
            fn analyze(&mut self, _ctx: &mut Context, _layer: &mut Layer) -> Result<Status> {
                let class = LayerBuilder::new(Token::from(1234)).build();
                let layer = Layer::new(&class, Slice::new());
                Ok(Status::Done(vec![layer]))
            }
        }

        #[derive(Clone)]
        struct TestDissector {}

        impl Dissector for TestDissector {
            fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
                assert_eq!(typ, "serial");
                Some(Box::new(TestWorker {}))
            }
        }

        let ctx = Context::new(HashMap::new());
        let mut diss = DissectorBox::new(TestDissector {});
        let mut worker = diss.new_worker("serial", &ctx).unwrap();

        let class = LayerBuilder::new(Token::null()).build();
        let mut layer = Layer::new(&class, Slice::new());
        let mut results = Vec::new();

        assert_eq!(
            worker.analyze(&mut ctx, &mut layer, &mut results).unwrap(),
            true
        );
        assert_eq!(results.len(), 1);
        let child = &results[0];
        assert_eq!(child.id(), Token::from(1234));
    }
}
