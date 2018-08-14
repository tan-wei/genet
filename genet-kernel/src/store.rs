use array_vec::ArrayVec;
use crossbeam_channel;
use decoder::{parallel, serial};
use filter::{self, Filter};
use frame::Frame;
use genet_abi::{fixed::MutFixed, layer::Layer};
use io::{Input, Output};
use profile::Profile;
use result::Result;
use std::{
    cmp,
    collections::HashMap,
    fmt,
    ops::Range,
    panic::{self, AssertUnwindSafe},
    sync::{Arc, RwLock},
    thread::{self, JoinHandle},
};

pub trait Callback: Send {
    fn on_frames_updated(&self, _frames: u32) {}
    fn on_filtered_frames_updated(&self, _id: u32, _frames: u32) {}
    fn on_output_done(&self, _id: u32, _error: Option<Box<::std::error::Error + Send>>) {}
    fn on_input_done(&self, _id: u32, _error: Option<Box<::std::error::Error + Send>>) {}
    fn on_error(&self, _error: Box<::std::error::Error + Send>) {}
}

#[derive(Debug)]
enum Command {
    PushFrames(Option<u32>, Result<Vec<MutFixed<Layer>>>),
    PushSerialFrames(Vec<Frame>),
    StoreFrames(Vec<Frame>),
    SetFilter(u32, Option<Box<Filter>>),
    PushOutput(u32, Box<Output>, Option<Box<Filter>>),
    Close,
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

type FrameStore = Arc<RwLock<ArrayVec<Frame>>>;
type FilteredFrameStore = Arc<RwLock<HashMap<u32, Vec<u32>>>>;

#[derive(Debug)]
pub struct Store {
    sender: crossbeam_channel::Sender<Command>,
    ev: EventLoop,
    frames: FrameStore,
    filtered: FilteredFrameStore,
    inputs: HashMap<u32, InputContext>,
    inputs_trash: Vec<InputContext>,
}

impl Store {
    pub fn new<C: 'static + Callback + Clone>(profile: Profile, callback: C) -> Store {
        let frames = Arc::new(RwLock::new(ArrayVec::new()));
        let filtered = Arc::new(RwLock::new(HashMap::new()));
        let (ev, send) = EventLoop::new(profile, callback, frames.clone(), filtered.clone());
        Store {
            sender: send,
            ev,
            frames,
            filtered,
            inputs: HashMap::new(),
            inputs_trash: Vec::new(),
        }
    }

    pub fn frames(&self, range: Range<usize>) -> Vec<*const Frame> {
        self.frames
            .read()
            .unwrap()
            .iter()
            .skip(range.start)
            .take(range.end.saturating_sub(range.start))
            .map(|f| f as *const Frame)
            .collect::<Vec<_>>()
    }

    pub fn filtered_frames(&self, id: u32, range: Range<usize>) -> Vec<u32> {
        let filtered = self.filtered.read().unwrap();
        if let Some(vec) = filtered.get(&id) {
            vec.iter()
                .skip(range.start)
                .take(range.end.saturating_sub(range.start))
                .cloned()
                .collect::<Vec<_>>()
        } else {
            Vec::new()
        }
    }

    pub fn len(&self) -> usize {
        let frames = self.frames.read().unwrap();
        frames.len()
    }

    pub fn set_filter(&mut self, id: u32, filter: Option<Box<Filter>>) {
        self.sender.send(Command::SetFilter(id, filter));
    }

    pub fn push_output<O: 'static + Output>(
        &mut self,
        id: u32,
        output: O,
        filter: Option<Box<Filter>>,
    ) {
        self.sender
            .send(Command::PushOutput(id, Box::new(output), filter));
    }

    pub fn set_input<I: 'static + Input>(&mut self, id: u32, input: I) {
        let holder = Arc::new(self.sender.clone());
        let sender = Arc::downgrade(&holder);
        let mut input = input;
        let handle = thread::spawn(move || {
            while let Some(sender) = sender.upgrade() {
                match input.read() {
                    Ok(layers) => {
                        if !layers.is_empty() {
                            sender.send(Command::PushFrames(Some(id), Ok(layers)));
                        }
                    }
                    Err(err) => {
                        let err = Error(err.description().to_string());
                        sender.send(Command::PushFrames(Some(id), Err(Box::new(err))));
                        break;
                    }
                }
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
        if let Some(mut input) = self.inputs.remove(&id) {
            input.holder = None;
            self.inputs_trash.push(input);
        }
    }
}

#[derive(Debug)]
struct InputContext {
    handle: Option<JoinHandle<()>>,
    holder: Option<Arc<crossbeam_channel::Sender<Command>>>,
}

impl Drop for InputContext {
    fn drop(&mut self) {
        self.holder = None;
        self.handle.take().unwrap().join().expect("failed to join");
    }
}

#[derive(Clone)]
struct ParallelCallback {
    sender: crossbeam_channel::Sender<Command>,
}

impl parallel::Callback for ParallelCallback {
    fn done(&self, result: Vec<Frame>) {
        self.sender.send(Command::PushSerialFrames(result));
    }
}

#[derive(Clone)]
struct SerialCallback {
    sender: crossbeam_channel::Sender<Command>,
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
    sender: crossbeam_channel::Sender<Command>,
}

impl EventLoop {
    const OUTPUT_BLOCK_SIZE: usize = 1024;

    pub fn new<C: 'static + Callback + Clone>(
        profile: Profile,
        callback: C,
        frames: FrameStore,
        filtered: FilteredFrameStore,
    ) -> (EventLoop, crossbeam_channel::Sender<Command>) {
        let (send, recv) = crossbeam_channel::unbounded();
        let sender = send.clone();
        let handle = thread::spawn(move || {
            let err_callback = callback.clone();
            let result = panic::catch_unwind(AssertUnwindSafe(move || {
                let mut filter_map = HashMap::new();
                let mut ppool = parallel::Pool::new(
                    &profile,
                    &ParallelCallback {
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
                                let len = {
                                    let mut frames = frames.write().unwrap();
                                    for f in vec {
                                        frames.push(f);
                                    }
                                    frames.len()
                                };
                                callback.on_frames_updated(len as u32);
                            }
                            Command::SetFilter(id, filter) => Self::process_push_filter(
                                id,
                                filter,
                                &filtered,
                                &mut filter_map,
                                &callback,
                            ),
                            Command::PushOutput(id, output, filter) => {
                                Self::process_output(id, output, filter, &frames, &callback)
                            }
                            Command::Close => return,
                        }
                    }
                    Self::process_filters(&frames, &filtered, &mut filter_map, &callback);
                }
            }));
            if let Err(err) = result {
                let message = if let Some(string) = err.downcast_ref::<String>() {
                    string
                } else if let Some(string) = err.downcast_ref::<&str>() {
                    string
                } else if let Some(err) = err.downcast_ref::<Error>() {
                    ::std::error::Error::description(err)
                } else {
                    "Thread Panicked"
                };
                let err = Error(message.to_string());
                err_callback.on_error(Box::new(err));
            }
        });
        let ev = EventLoop {
            handle: Some(handle),
            sender: send.clone(),
        };
        (ev, send)
    }

    fn process_input(
        id: Option<u32>,
        result: Result<Vec<MutFixed<Layer>>>,
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
                        }).collect::<Vec<_>>();
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

    fn process_output(
        id: u32,
        output: Box<Output>,
        filter: Option<Box<Filter>>,
        frames: &FrameStore,
        callback: &Callback,
    ) {
        let frames = frames.read().unwrap();
        let mut offset = 0;
        {
            let filter = filter.map(|f| f.new_worker());
            let mut output = output;
            while offset < frames.len() {
                let len = cmp::min(frames.len() - offset, Self::OUTPUT_BLOCK_SIZE);
                let frames = frames
                    .iter()
                    .skip(offset)
                    .take(len)
                    .filter(|frame| filter.as_ref().map_or(true, |f| f.test(frame)))
                    .collect::<Vec<_>>();
                if let Err(err) = output.write(frames.as_slice()) {
                    let err = Error(err.description().to_string());
                    callback.on_output_done(id, Some(Box::new(err)));
                    return;
                }
                offset += len;
            }
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
        }
        filtered.write().unwrap().remove(&id);
    }

    fn process_filters(
        frames: &FrameStore,
        filtered: &FilteredFrameStore,
        filter_map: &mut HashMap<u32, FilterContext>,
        callback: &Callback,
    ) {
        for (id, filter) in filter_map.iter_mut() {
            let mut indices: Vec<u32> = {
                let frames = frames.read().unwrap();
                let mut indeces = frames
                    .iter()
                    .skip(filter.offset)
                    .filter_map(|frame| {
                        if filter.worker.test(frame) {
                            Some(frame.index())
                        } else {
                            None
                        }
                    }).collect::<Vec<_>>();
                filter.offset = frames.len();
                indeces
            };
            let len = if !indices.is_empty() {
                let mut filtered = filtered.write().unwrap();
                let mut frames = filtered.entry(*id).or_insert_with(Vec::new);
                frames.append(&mut indices);
                frames.len()
            } else {
                0
            };
            if len > 0 {
                callback.on_filtered_frames_updated(*id, len as u32);
            }
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
    use filter;
    use profile::Profile;
    use store::{Callback, Store};

    #[derive(Clone)]
    struct TestCallback {}
    impl Callback for TestCallback {}

    struct TestFilterWorker {}
    impl filter::Worker for TestFilterWorker {}

    #[derive(Debug)]
    struct TestFilter {}
    impl filter::Filter for TestFilter {
        fn new_worker(&self) -> Box<filter::Worker> {
            Box::new(TestFilterWorker {})
        }
    }

    #[test]
    fn drop() {
        let profile = Profile::new();
        let _store = Store::new(profile, TestCallback {});
    }

    #[test]
    fn invalid_range() {
        let profile = Profile::new();
        let mut store = Store::new(profile, TestCallback {});
        store.set_filter(0, Some(Box::new(TestFilter {})));
        assert_eq!(store.frames(100..0).len(), 0);
        assert_eq!(store.filtered_frames(0, 100..0).len(), 0);
    }
}
