use crate::{
    array_vec::ArrayVec,
    decoder::{parallel, serial},
    frame::Frame,
    io::{Input, Output},
    profile::Profile,
    result::Result,
};
use crossbeam_channel;
use failure::{format_err, Error};
use fnv::FnvHashMap;
use genet_abi::{
    filter::{LayerContext, LayerFilter},
    fixed::MutFixed,
    layer::Layer,
};
use genet_filter::CompiledLayerFilter;
use parking_lot::RwLock;
use std::{
    fmt,
    ops::Range,
    panic::{self, AssertUnwindSafe},
    sync::Arc,
    thread::{self, JoinHandle},
};

const OUTPUT_BLOCK_SIZE: usize = 2048;
const MAX_FILTER_SIZE: usize = 2048;

pub trait Callback: Send {
    fn on_frames_updated(&self, _frames: u32) {}
    fn on_async_frames_updated(&self, _frames: u32) {}
    fn on_filtered_frames_updated(&self, _id: u32, _frames: u32) {}
    fn on_output_done(&self, _id: u32, _error: Option<Error>) {}
    fn on_input_done(&self, _id: u32, _error: Option<Error>) {}
    fn on_error(&self, _error: Error) {}
}

#[derive(Debug)]
enum Command {
    PushFrames(Option<u32>, Result<Vec<MutFixed<Layer>>>),
    PushSerialFrames(Vec<Frame>),
    StoreFrames(Vec<Frame>),
    SetFilter(u32, Option<CompiledLayerFilter>),
    PushOutput(u32, Box<Output>, Option<CompiledLayerFilter>),
    Close,
}

type FrameStore = Arc<RwLock<ArrayVec<Frame>>>;
type FilteredFrameStore = Arc<RwLock<FnvHashMap<u32, Vec<u32>>>>;

#[derive(Debug)]
pub struct Store {
    sender: crossbeam_channel::Sender<Command>,
    ev: EventLoop,
    frames: FrameStore,
    filtered: FilteredFrameStore,
    inputs: FnvHashMap<u32, InputContext>,
    inputs_trash: Vec<InputContext>,
}

impl Store {
    pub fn new<C: 'static + Callback + Clone>(profile: Profile, callback: C) -> Store {
        let frames = Arc::new(RwLock::new(ArrayVec::new()));
        let filtered = Arc::new(RwLock::new(FnvHashMap::default()));
        let (send, recv) = crossbeam_channel::unbounded();
        let ev = EventLoop::new(
            profile,
            callback,
            frames.clone(),
            filtered.clone(),
            &send,
            recv,
        );
        Store {
            sender: send,
            ev,
            frames,
            filtered,
            inputs: FnvHashMap::default(),
            inputs_trash: Vec::new(),
        }
    }

    pub fn frames(&self, range: Range<usize>) -> Vec<*const Frame> {
        self.frames
            .read()
            .iter()
            .skip(range.start)
            .take(range.end.saturating_sub(range.start))
            .map(|f| f as *const Frame)
            .collect::<Vec<_>>()
    }

    pub fn filtered_frames(&self, id: u32, range: Range<usize>) -> Vec<u32> {
        let filtered = self.filtered.read();
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
        let frames = self.frames.read();
        frames.len()
    }

    pub fn set_filter(&mut self, id: u32, filter: Option<CompiledLayerFilter>) {
        let _ = self.sender.send(Command::SetFilter(id, filter));
    }

    pub fn push_output<O: 'static + Output>(
        &mut self,
        id: u32,
        output: O,
        filter: Option<CompiledLayerFilter>,
    ) {
        let _ = self
            .sender
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
                            let _ = sender.send(Command::PushFrames(Some(id), Ok(layers)));
                        }
                    }
                    Err(err) => {
                        let _ = sender.send(Command::PushFrames(Some(id), Err(err)));
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
        let _ = self.sender.send(Command::PushSerialFrames(result));
    }
}

#[derive(Clone)]
struct SerialCallback {
    sender: crossbeam_channel::Sender<Command>,
}

impl serial::Callback for SerialCallback {
    fn done(&self, result: Vec<Frame>) {
        let _ = self.sender.send(Command::StoreFrames(result));
    }
}

struct FilterContext {
    filter: CompiledLayerFilter,
    offset: usize,
}

struct EventLoop {
    handle: Option<JoinHandle<()>>,
    sender: crossbeam_channel::Sender<Command>,
}

impl EventLoop {
    pub fn new<C: 'static + Callback + Clone>(
        profile: Profile,
        callback: C,
        frames: FrameStore,
        filtered: FilteredFrameStore,
        send: &crossbeam_channel::Sender<Command>,
        recv: crossbeam_channel::Receiver<Command>,
    ) -> EventLoop {
        let sender = send.clone();
        let handle = thread::spawn(move || {
            let err_callback = callback.clone();
            let result = panic::catch_unwind(AssertUnwindSafe(move || {
                let mut filter_map = FnvHashMap::default();
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
                callback.on_async_frames_updated(0);
                loop {
                    if let Ok(cmd) = recv.recv() {
                        match cmd {
                            Command::PushFrames(id, result) => {
                                Self::process_input(id, result, &mut cnt, &mut ppool, &callback)
                            }
                            Command::PushSerialFrames(vec) => {
                                spool.process(vec);
                            }
                            Command::StoreFrames(vec) => {
                                let len = {
                                    let mut frames = frames.write();
                                    for f in vec {
                                        frames.push(f);
                                    }
                                    frames.len()
                                };
                                callback.on_frames_updated(len as u32);
                                callback.on_async_frames_updated(len as u32);
                            }
                            Command::SetFilter(id, filter) => Self::process_push_filter(
                                id,
                                filter,
                                &filtered,
                                &mut filter_map,
                                &callback,
                            ),
                            Command::PushOutput(id, output, filter) => {
                                Self::process_output(id, output, &filter, &frames, &callback)
                            }
                            Command::Close => return,
                        }
                    }
                    Self::process_filters(&frames, &filtered, &mut filter_map, &callback);
                }
            }));
            if let Err(err) = result {
                err_callback.on_error(format_err!("{:?}", err));
            }
        });
        EventLoop {
            handle: Some(handle),
            sender: send.clone(),
        }
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

    fn process_output(
        id: u32,
        output: Box<Output>,
        filter: &Option<CompiledLayerFilter>,
        frames: &FrameStore,
        callback: &Callback,
    ) {
        let frames = frames.read();
        let mut offset = 0;
        {
            let mut output = output;
            while offset < frames.len() {
                let len = OUTPUT_BLOCK_SIZE.min(frames.len() - offset);
                let frames = frames
                    .iter()
                    .skip(offset)
                    .take(len)
                    .filter(|frame| {
                        let ctx = LayerContext::new(frame.layers());
                        filter.as_ref().map_or(true, |f| f.test(&ctx))
                    })
                    .collect::<Vec<_>>();
                if let Err(err) = output.write(frames.as_slice()) {
                    callback.on_output_done(id, Some(err));
                    return;
                }
                offset += len;
            }
            if let Err(err) = output.end() {
                callback.on_output_done(id, Some(err));
                return;
            }
        }
        callback.on_output_done(id, None);
    }

    fn process_push_filter(
        id: u32,
        filter: Option<CompiledLayerFilter>,
        filtered: &FilteredFrameStore,
        filter_map: &mut FnvHashMap<u32, FilterContext>,
        callback: &Callback,
    ) {
        if let Some(filter) = filter {
            filter_map.insert(
                id,
                FilterContext {
                    offset: 0,
                    filter: filter.clone(),
                },
            );
            callback.on_filtered_frames_updated(id, 0);
        } else {
            filter_map.remove(&id);
        }
        filtered.write().remove(&id);
    }

    fn process_filters(
        frames: &FrameStore,
        filtered: &FilteredFrameStore,
        filter_map: &mut FnvHashMap<u32, FilterContext>,
        callback: &Callback,
    ) {
        for (id, fctx) in filter_map.iter_mut() {
            loop {
                let (mut indices, end) = {
                    let frames = frames.read();
                    let indices = frames
                        .iter()
                        .skip(fctx.offset)
                        .take(MAX_FILTER_SIZE)
                        .filter_map(|frame| {
                            let ctx = LayerContext::new(frame.layers());
                            if fctx.filter.test(&ctx) {
                                Some(frame.index())
                            } else {
                                None
                            }
                        })
                        .collect::<Vec<_>>();
                    fctx.offset = frames.len().min(fctx.offset + MAX_FILTER_SIZE);
                    (indices, fctx.offset >= frames.len())
                };
                if !indices.is_empty() {
                    let len = {
                        let mut filtered = filtered.write();
                        let frames = filtered.entry(*id).or_insert_with(Vec::new);
                        frames.append(&mut indices);
                        frames.len()
                    };
                    callback.on_filtered_frames_updated(*id, len as u32);
                }
                if end {
                    break;
                }
            }
        }
    }
}

impl Drop for EventLoop {
    fn drop(&mut self) {
        let _ = self.sender.send(Command::Close);
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
    use crate::{
        profile::Profile,
        store::{Callback, Store},
    };
    use genet_filter::CompiledLayerFilter;

    #[derive(Clone)]
    struct TestCallback {}
    impl Callback for TestCallback {}

    #[test]
    fn drop() {
        let profile = Profile::new();
        let _store = Store::new(profile, TestCallback {});
    }

    #[test]
    fn invalid_range() {
        let profile = Profile::new();
        let mut store = Store::new(profile, TestCallback {});
        store.set_filter(0, CompiledLayerFilter::compile("false").ok());
        assert_eq!(store.frames(100..0).len(), 0);
        assert_eq!(store.filtered_frames(0, 100..0).len(), 0);
    }
}
