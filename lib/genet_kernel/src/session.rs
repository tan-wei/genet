use frame::Frame;
use genet_ffi::dissector::Dissector;
use genet_ffi::layer::Layer;
use genet_ffi::ptr::MutPtr;
use profile::Profile;
use serde::ser::{Serialize, SerializeMap, Serializer};
use std::ops::Range;
use store::{self, Store};

#[derive(Debug)]
pub struct Session {
    store: Store,
    profile: Profile,
}

pub struct Context {
    profile: Profile,
    close_stream: bool,
}

impl Context {
    pub fn new(profile: Profile) -> Context {
        Context {
            profile,
            close_stream: false,
        }
    }

    pub fn check_close_stream(&mut self) -> bool {
        let close = self.close_stream;
        self.close_stream = false;
        close
    }
}

impl Session {
    pub fn new<C: 'static + Callback>(profile: Profile, callback: C) -> Session {
        Session {
            store: Store::new(
                profile.clone(),
                StoreCallback {
                    callback: Box::new(callback),
                },
            ),
            profile,
        }
    }

    pub fn context(&self) -> Context {
        Context::new(self.profile.clone())
    }

    pub fn push_frames(&mut self, layers: Vec<MutPtr<Layer>>) {
        self.store.process(layers);
    }

    pub fn frames(&self, range: Range<usize>) -> Vec<*const Frame> {
        self.store.frames(range)
    }

    pub fn filtered_frames(&self, id: u32, range: Range<usize>) -> Vec<*const Frame> {
        self.store.filtered_frames(id, range)
    }

    pub fn len(&self) -> usize {
        self.store.len()
    }
}

struct StoreCallback {
    callback: Box<Callback>,
}

impl store::Callback for StoreCallback {
    fn on_frames_updated(&self, frames: u32) {
        self.callback.on_event(Event::Frames(frames));
    }
    fn on_filtered_frames_updated(&self, id: u32, frames: u32) {
        self.callback.on_event(Event::FilteredFrames(id, frames));
    }
    fn on_output_done(&self, id: u32, error: Option<::std::io::Error>) {
        self.callback.on_event(Event::Input(id, error));
    }
    fn on_input_done(&self, id: u32, error: Option<::std::io::Error>) {
        self.callback.on_event(Event::Output(id, error));
    }
}

#[derive(Debug)]
pub enum Event {
    Frames(u32),
    FilteredFrames(u32, u32),
    Input(u32, Option<::std::io::Error>),
    Output(u32, Option<::std::io::Error>),
}

pub trait Callback: Send {
    fn on_event(&self, event: Event);
}

impl Serialize for Event {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match self {
            &Event::Frames(len) => {
                let mut s = serializer.serialize_map(Some(2))?;
                s.serialize_entry("type", "frames")?;
                s.serialize_entry("length", &len)?;
                s.end()
            }
            &Event::FilteredFrames(id, len) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "filtered_frames")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("length", &len)?;
                s.end()
            }
            &Event::Input(ref id, ref err) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "input")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("error", &err.as_ref().map(|e| format!("{}", e)))?;
                s.end()
            }
            &Event::Output(ref id, ref err) => {
                let mut s = serializer.serialize_map(Some(3))?;
                s.serialize_entry("type", "output")?;
                s.serialize_entry("id", &id)?;
                s.serialize_entry("error", &err.as_ref().map(|e| format!("{}", e)))?;
                s.end()
            }
        }
    }
}
