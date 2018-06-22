use array_vec::ArrayVec;
use chan;
use dissector::{parallel, serial};
use filter::{self, Filter};
use frame::Frame;
use genet_abi::result::Result;
use genet_abi::{context::Context, layer::Layer, ptr::MutPtr, token::Token};
use io::{Input, Output};
use profile::Profile;
use std::{
    cmp,
    collections::HashMap,
    fmt,
    ops::Range,
    sync::{Arc, Mutex, Weak},
    thread::{self, JoinHandle},
};

pub trait Callback: Send {
    fn on_frames_updated(&self, _frames: u32) {}
    fn on_filtered_frames_updated(&self, _id: u32, _frames: u32) {}
    fn on_output_done(&self, _id: u32, _error: Option<Box<::std::error::Error + Send>>) {}
    fn on_input_done(&self, _id: u32, _error: Option<Box<::std::error::Error + Send>>) {}
}

#[derive(Debug)]
enum Command {
    PushFrames(Option<u32>, Result<Vec<MutPtr<Layer>>>),
    PushSerialFrames(Vec<Frame>),
    StoreFrames(Vec<Frame>),
    SetFilter(u32, Option<Box<Filter>>),
    PushOutput(u32, Box<Output>),
    Close,
}

type FrameStore = Arc<Mutex<ArrayVec<Frame>>>;
type FilteredFrameStore = Arc<Mutex<HashMap<u32, Vec<u32>>>>;

#[derive(Debug)]
pub struct Store {
    sender: chan::Sender<Command>,
    ev: EventLoop,
    frames: FrameStore,
    filtered: FilteredFrameStore,
    inputs: HashMap<u32, InputContext>,
}

impl Store {
    pub fn new<C: 'static + Callback>(profile: Profile, callback: C) -> Store {
        let frames = Arc::new(Mutex::new(ArrayVec::new()));
        let filtered = Arc::new(Mutex::new(HashMap::new()));
        let (ev, send) = EventLoop::new(profile, callback, frames.clone(), filtered.clone());
        Store {
            sender: send,
            ev,
            frames,
            filtered,
            inputs: HashMap::new(),
        }
    }

    pub fn process(&self, layers: Vec<MutPtr<Layer>>) {
        if !layers.is_empty() {
            self.sender.send(Command::PushFrames(None, Ok(layers)));
        }
    }

    pub fn frames(&self, range: Range<usize>) -> Vec<*const Frame> {
        let frames = self.frames.lock().unwrap();
        frames
            .iter()
            .skip(range.start)
            .take(range.end - range.start)
            .map(|f| f as *const Frame)
            .collect::<Vec<_>>()
    }

    pub fn filtered_frames(&self, id: u32, range: Range<usize>) -> Vec<*const Frame> {
        let filtered = self.filtered.lock().unwrap();
        let frames = self.frames.lock().unwrap();
        if let Some(vec) = filtered.get(&id) {
            vec.iter()
                .skip(range.start)
                .take(range.end - range.start)
                .map(|i| frames.get(*i as usize).unwrap() as *const Frame)
                .collect::<Vec<_>>()
        } else {
            Vec::new()
        }
    }

    pub fn len(&self) -> usize {
        let frames = self.frames.lock().unwrap();
        frames.len()
    }

    pub fn set_filter<F: 'static + Filter>(&mut self, id: u32, filter: F) {
        self.sender
            .send(Command::SetFilter(id, Some(Box::new(filter))));
    }

    pub fn unset_filter(&mut self, id: u32) {
        self.sender.send(Command::SetFilter(id, None));
    }

    pub fn push_output<O: 'static + Output>(&mut self, id: u32, output: O) {
        self.sender.send(Command::PushOutput(id, Box::new(output)));
    }

    pub fn set_input<I: 'static + Input>(&mut self, id: u32, input: I) {
        let holder = Arc::new(self.sender.clone());
        let sender = Arc::downgrade(&holder);
        let mut input = input;
        let handle = thread::spawn(move || loop {
            if let Some(sender) = sender.upgrade() {
                match input.read() {
                    Ok(layers) => {
                        if !layers.is_empty() {
                            sender.send(Command::PushFrames(Some(id), Ok(layers)));
                        }
                    }
                    _ => break,
                }
            } else {
                break;
            }
        });
        self.inputs.insert(
            id,
            InputContext {
                handle: Some(handle),
                holder: Some(holder),
            },
        );
    }

    pub fn unset_input(&mut self, id: u32) {
        self.inputs.remove(&id);
    }
}

#[derive(Debug)]
struct InputContext {
    handle: Option<JoinHandle<()>>,
    holder: Option<Arc<chan::Sender<Command>>>,
}

impl Drop for InputContext {
    fn drop(&mut self) {
        self.holder = None;
        self.handle.take().unwrap().join().expect("failed to join");
    }
}

#[derive(Clone)]
struct ParallelCallback {
    sender: chan::Sender<Command>,
}

impl parallel::Callback for ParallelCallback {
    fn done(&self, result: Vec<Frame>) {
        self.sender.send(Command::PushSerialFrames(result));
    }
}

#[derive(Clone)]
struct SerialCallback {
    sender: chan::Sender<Command>,
}

impl serial::Callback for SerialCallback {
    fn done(&self, result: Vec<Frame>) {
        self.sender.send(Command::StoreFrames(result));
    }
}

struct FilterContext {
    worker: Box<filter::Worker>,
    offset: usize,
}

struct EventLoop {
    handle: Option<JoinHandle<()>>,
    sender: chan::Sender<Command>,
}

impl EventLoop {
    pub fn new<C: 'static + Callback>(
        profile: Profile,
        callback: C,
        frames: FrameStore,
        filtered: FilteredFrameStore,
    ) -> (EventLoop, chan::Sender<Command>) {
        let (send, recv) = chan::async();
        let sender = send.clone();
        let handle = thread::spawn(move || {
            let mut filter_map = HashMap::new();
            let mut ppool = parallel::Pool::new(
                profile.clone(),
                ParallelCallback {
                    sender: sender.clone(),
                },
            );
            let mut spool = serial::Pool::new(
                profile.clone(),
                SerialCallback {
                    sender: sender.clone(),
                },
            );
            let mut cnt = 0;
            callback.on_frames_updated(0);
            loop {
                if let Some(cmd) = recv.recv() {
                    match cmd {
                        Command::PushFrames(id, result) => {
                            Self::process_input(id, result, &mut cnt, &mut ppool, &callback)
                        }
                        Command::PushSerialFrames(vec) => {
                            spool.process(vec);
                        }
                        Command::StoreFrames(mut vec) => {
                            let mut frames = frames.lock().unwrap();
                            for f in vec.into_iter() {
                                frames.push(f);
                            }
                            callback.on_frames_updated(frames.len() as u32);
                        }
                        Command::SetFilter(id, filter) => Self::process_push_filter(
                            id,
                            filter,
                            &filtered,
                            &mut filter_map,
                            &callback,
                        ),
                        Command::PushOutput(id, output) => {
                            Self::process_output(id, output, &frames, &callback)
                        }
                        Command::Close => return,
                    }
                }
                Self::process_filters(&frames, &filtered, &mut filter_map, &callback);
            }
        });
        let ev = EventLoop {
            handle: Some(handle),
            sender: send.clone(),
        };
        (ev, send)
    }

    const OUTPUT_BLOCK_SIZE: usize = 1024;

    fn process_input(
        id: Option<u32>,
        result: Result<Vec<MutPtr<Layer>>>,
        cnt: &mut u32,
        pool: &mut parallel::Pool,
        callback: &Callback,
    ) {
        match result {
            Ok(layers) => {
                if layers.is_empty() {
                    if let Some(id) = id {
                        callback.on_input_done(id, None);
                    }
                } else {
                    let frames = layers
                        .into_iter()
                        .enumerate()
                        .map(|(i, root)| {
                            let index = *cnt + i as u32;
                            Frame::new(index, root)
                        })
                        .collect::<Vec<_>>();
                    *cnt += frames.len() as u32;
                    pool.process(frames);
                }
            }
            Err(err) => {
                if let Some(id) = id {
                    callback.on_input_done(id, Some(err));
                }
            }
        }
    }

    fn process_output(id: u32, mut output: Box<Output>, frames: &FrameStore, callback: &Callback) {
        let frames = frames.lock().unwrap();
        let mut offset = 0;
        while offset < frames.len() {
            let len = cmp::min(frames.len() - offset, Self::OUTPUT_BLOCK_SIZE);
            let frames = frames.iter().skip(offset).take(len).collect::<Vec<_>>();
            if let Err(err) = output.write(Some(frames.as_slice())) {
                callback.on_output_done(id, Some(err));
                return;
            }
            offset += len;
        }
        if let Err(err) = output.write(None) {
            callback.on_output_done(id, Some(err));
            return;
        }
        callback.on_output_done(id, None);
    }

    fn process_push_filter(
        id: u32,
        filter: Option<Box<Filter>>,
        filtered: &FilteredFrameStore,
        filter_map: &mut HashMap<u32, FilterContext>,
        callback: &Callback,
    ) {
        if let Some(filter) = filter {
            filter_map.insert(
                id,
                FilterContext {
                    worker: filter.new_worker(),
                    offset: 0,
                },
            );
            callback.on_filtered_frames_updated(id, 0);
        } else {
            filter_map.remove(&id);
            filtered.lock().unwrap().remove(&id);
        }
    }

    fn process_filters(
        frames: &FrameStore,
        filtered: &FilteredFrameStore,
        filter_map: &mut HashMap<u32, FilterContext>,
        callback: &Callback,
    ) {
        let frames = frames.lock().unwrap();
        for (id, filter) in filter_map.iter_mut() {
            let mut indices = frames
                .iter()
                .skip(filter.offset)
                .filter_map(|frame| {
                    if filter.worker.test(frame) {
                        Some(frame.index())
                    } else {
                        None
                    }
                })
                .collect::<Vec<_>>();
            if !indices.is_empty() {
                let mut filtered = filtered.lock().unwrap();
                let mut frames = filtered.entry(*id).or_insert_with(|| Vec::new());
                frames.append(&mut indices);
                callback.on_filtered_frames_updated(*id, frames.len() as u32);
            }
            filter.offset = frames.len();
        }
    }
}

impl Drop for EventLoop {
    fn drop(&mut self) {
        self.sender.send(Command::Close);
        self.handle.take().unwrap().join().expect("failed to join");
    }
}

impl fmt::Debug for EventLoop {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "EventLoop")
    }
}

#[cfg(test)]
mod tests {
    use profile::Profile;
    use session::Context;
    use store::{Callback, Store};

    struct TestCallback {}
    impl Callback for TestCallback {}

    #[test]
    fn drop() {
        let profile = Profile::new();
        let _store = Store::new(profile, TestCallback {});
    }
}
