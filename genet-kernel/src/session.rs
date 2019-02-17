use crate::{
    frame::Frame,
    io::{Input, Output},
    profile::Profile,
    store::{self, Store},
};
use failure::Error;
use genet_abi::{
    self,
    attr::AttrClass,
    fixed::{Fixed, MutFixed},
    layer::{Layer, LayerClass},
    reader,
    token::Token,
    writer,
};
use genet_filter::CompiledLayerFilter;
use serde::ser::{Serialize, SerializeMap, Serializer};
use std::ops::Range;

pub struct Session {
    store: Store,
    callback: Box<Callback>,
    profile: Profile,
    io_cnt: u32,
}

impl Session {
    pub fn new<C: 'static + Callback + Clone>(profile: Profile, callback: C) -> Session {
        Session {
            store: Store::new(
                profile.clone(),
                StoreCallback {
                    callback: Box::new(callback.clone()),
                },
            ),
            callback: Box::new(callback),
            profile,
            io_cnt: 0,
        }
    }

    pub fn frames(&self, range: Range<usize>) -> Vec<*const Frame> {
        self.store.frames(range)
    }

    pub fn filtered_frames(&self, id: u32, range: Range<usize>) -> Vec<u32> {
        self.store.filtered_frames(id, range)
    }

    pub fn set_filter(&mut self, id: u32, filter: Option<CompiledLayerFilter>) {
        self.store.set_filter(id, filter);
    }

    pub fn create_reader(&mut self, id: &str, url: &str) -> u32 {
        if let Some(reader) = self.profile.readers().find(|&r| r.id.as_str() == id) {
            self.io_cnt += 1;
            let ctx = self.profile.context();
            match reader.reader.new_worker(&ctx, url) {
                Ok(input) => {
                    let id: Token = input.layer_id().to_string().into();
                    self.store
                        .set_input(self.io_cnt, WorkerInput::new(input, id));
                    return self.io_cnt;
                }
                Err(err) => {
                    self.callback.on_event(Event::Error(err));
                }
            }
        }
        0
    }

    pub fn create_writer(
        &mut self,
        id: &str,
        url: &str,
        filter: Option<CompiledLayerFilter>,
    ) -> u32 {
        if let Some(writer) = self.profile.writers().find(|&r| r.id.as_str() == id) {
            self.io_cnt += 1;
            let ctx = self.profile.context();
            match writer.writer.new_worker(&ctx, url) {
                Ok(output) => {
                    self.store
                        .push_output(self.io_cnt, WorkerOutput::new(output), filter);
                    return self.io_cnt;
                }
                Err(err) => {
                    self.callback.on_event(Event::Error(err));
                }
            }
        }
        0
    }

    pub fn close_reader(&mut self, handle: u32) {
        self.store.unset_input(handle);
    }

    pub fn len(&self) -> usize {
        self.store.len()
    }

    pub fn profile(&self) -> &Profile {
        &self.profile
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

#[derive(Clone)]
struct StoreCallback {
    callback: Box<Callback>,
}

impl store::Callback for StoreCallback {
    fn on_frames_updated(&self, frames: u32) {
        self.callback.on_event(Event::Frames(frames));
    }

    fn on_async_frames_updated(&self, frames: u32) {
        self.callback.on_event(Event::AsyncFrames(frames));
    }

    fn on_filtered_frames_updated(&self, id: u32, frames: u32) {
        self.callback.on_event(Event::FilteredFrames(id, frames));
    }

    fn on_output_done(&self, id: u32, error: Option<Error>) {
        self.callback.on_event(Event::Output(id, error));
    }

    fn on_input_done(&self, id: u32, error: Option<Error>) {
        self.callback.on_event(Event::Input(id, error));
    }

    fn on_error(&self, error: Error) {
        self.callback.on_event(Event::Error(error));
    }
}

#[derive(Debug)]
pub enum Event {
    Frames(u32),
    AsyncFrames(u32),
    FilteredFrames(u32, u32),
    Input(u32, Option<Error>),
    Output(u32, Option<Error>),
    Error(Error),
}

pub trait Callback: CallbackClone + Send {
    fn on_event(&self, event: Event);
}

pub trait CallbackClone {
    fn clone_box(&self) -> Box<Callback>;
}

impl<T> CallbackClone for T
where
    T: 'static + Callback + Clone,
{
    fn clone_box(&self) -> Box<Callback> {
        Box::new(self.clone())
    }
}

impl Clone for Box<Callback> {
    fn clone(&self) -> Box<Callback> {
        self.clone_box()
    }
}

impl Serialize for Event {
    fn serialize<S>(&self, serializer: S) -> std::result::Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match self {
            Event::Frames(len) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "frames")?;
                s.serialize_entry("length", &len)?;
                s.end()
            }
            Event::AsyncFrames(len) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "async_frames")?;
                s.serialize_entry("length", &len)?;
                s.end()
            }
            Event::FilteredFrames(id, len) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "filtered_frames")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("length", &len)?;
                s.end()
            }
            Event::Input(id, err) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "input")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("error", &err.as_ref().map(|e| format!("{}", e)))?;
                s.end()
            }
            Event::Output(id, err) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "output")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("error", &err.as_ref().map(|e| format!("{}", e)))?;
                s.end()
            }
            Event::Error(err) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "error")?;
                s.serialize_entry("error", &format!("{}", err))?;
                s.end()
            }
        }
    }
}

#[derive(Debug)]
struct WorkerOutput {
    worker: writer::WorkerBox,
}

impl WorkerOutput {
    fn new(worker: writer::WorkerBox) -> WorkerOutput {
        Self { worker }
    }
}

impl Output for WorkerOutput {
    fn write(&mut self, frames: &[&Frame]) -> genet_abi::result::Result<()> {
        for frame in frames.iter() {
            self.worker.write(frame.index(), &frame.layers()[0])?;
        }
        Ok(())
    }

    fn end(&mut self) -> genet_abi::result::Result<()> {
        self.worker.end()
    }
}

#[derive(Debug)]
struct WorkerInput {
    worker: reader::WorkerBox,
    class: Box<Fixed<LayerClass>>,
}

impl WorkerInput {
    fn new(worker: reader::WorkerBox, id: Token) -> WorkerInput {
        let attr = vec![Fixed::new(AttrClass::builder(id).build())];
        Self {
            worker,
            class: Box::new(Fixed::new(LayerClass::builder(attr).build())),
        }
    }
}

impl Input for WorkerInput {
    fn read(&mut self) -> genet_abi::result::Result<Vec<MutFixed<Layer>>> {
        self.worker.read().map(|slices| {
            slices
                .into_iter()
                .map(|data| MutFixed::new(Layer::new(&self.class, &data)))
                .collect()
        })
    }
}
