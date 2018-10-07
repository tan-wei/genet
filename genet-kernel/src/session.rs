use filter::Filter;
use frame::Frame;
use genet_abi::{
    self,
    fixed::MutFixed,
    io::{ReaderWorkerBox, WriterWorkerBox},
    layer::Layer,
};
use io::{Input, Output};
use profile::Profile;
use serde::ser::{Serialize, SerializeMap, Serializer};
use std::{fmt, ops::Range};
use store::{self, Store};

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

    pub fn set_filter(&mut self, id: u32, filter: Option<Filter>) {
        self.store.set_filter(id, filter);
    }

    pub fn create_reader(&mut self, id: &str, arg: &str) -> u32 {
        if let Some(reader) = self.profile.readers().find(|&&r| r.id().as_str() == id) {
            self.io_cnt += 1;
            let ctx = self.profile.context();
            match reader.new_worker(&ctx, arg) {
                Ok(input) => {
                    self.store
                        .set_input(self.io_cnt, ReaderWorkerInput::new(input));
                    return self.io_cnt;
                }
                Err(err) => {
                    let err = Error(err.description().to_string());
                    self.callback.on_event(Event::Error(Box::new(err)));
                }
            }
        }
        0
    }

    pub fn create_writer(&mut self, id: &str, arg: &str, filter: Option<Filter>) -> u32 {
        if let Some(writer) = self.profile.writers().find(|&&r| r.id().as_str() == id) {
            self.io_cnt += 1;
            let ctx = self.profile.context();
            match writer.new_worker(&ctx, arg) {
                Ok(output) => {
                    self.store
                        .push_output(self.io_cnt, WriterWorkerOutput::new(output), filter);
                    return self.io_cnt;
                }
                Err(err) => {
                    let err = Error(err.description().to_string());
                    self.callback.on_event(Event::Error(Box::new(err)));
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

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

#[derive(Debug)]
struct Error(String);

impl ::std::error::Error for Error {
    fn description(&self) -> &str {
        &self.0
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0)
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

    fn on_output_done(&self, id: u32, error: Option<Box<::std::error::Error + Send>>) {
        self.callback.on_event(Event::Output(id, error));
    }

    fn on_input_done(&self, id: u32, error: Option<Box<::std::error::Error + Send>>) {
        self.callback.on_event(Event::Input(id, error));
    }

    fn on_error(&self, error: Box<::std::error::Error + Send>) {
        self.callback.on_event(Event::Error(error));
    }
}

#[derive(Debug)]
pub enum Event {
    Frames(u32),
    AsyncFrames(u32),
    FilteredFrames(u32, u32),
    Input(u32, Option<Box<::std::error::Error + Send>>),
    Output(u32, Option<Box<::std::error::Error + Send>>),
    Error(Box<::std::error::Error + Send>),
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
    fn serialize<S>(&self, serializer: S) -> ::std::result::Result<S::Ok, S::Error>
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
struct WriterWorkerOutput {
    worker: WriterWorkerBox,
}

impl WriterWorkerOutput {
    fn new(worker: WriterWorkerBox) -> WriterWorkerOutput {
        Self { worker }
    }
}

impl Output for WriterWorkerOutput {
    fn write(&mut self, frames: &[&Frame]) -> genet_abi::result::Result<()> {
        for frame in frames.iter() {
            for layer in frame.layers() {
                // FIXME
                //self.worker.write(frame.index(), &[layer])?;
            }
        }
        Ok(())
    }

    fn end(&mut self) -> genet_abi::result::Result<()> {
        self.worker.end()
    }
}

#[derive(Debug)]
struct ReaderWorkerInput {
    worker: ReaderWorkerBox,
}

impl ReaderWorkerInput {
    fn new(worker: ReaderWorkerBox) -> ReaderWorkerInput {
        Self { worker }
    }
}

impl Input for ReaderWorkerInput {
    fn read(&mut self) -> genet_abi::result::Result<Vec<MutFixed<Layer>>> {
        self.worker.read()
    }
}
