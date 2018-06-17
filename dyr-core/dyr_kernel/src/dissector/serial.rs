use chan;
use dissector::dispatcher::Dispatcher;
use dyr_ffi::layer::Layer;
use frame::Frame;
use profile::Profile;
use std::collections::BTreeMap;
use std::thread::{self, JoinHandle};

pub trait Callback: Sync + Send {
    fn done(&self, result: Vec<Frame>);
}

pub struct Pool {
    sender: chan::Sender<Option<Vec<Frame>>>,
    handles: Vec<JoinHandle<()>>,
}

impl Pool {
    pub fn new<C: 'static + Callback>(profile: Profile, callback: C) -> Pool {
        let callback = Box::new(callback);
        let (send, recv) = chan::async::<Option<Vec<Frame>>>();
        let mut handles = Vec::new();

        let handle = thread::spawn(move || {
            let mut disp = Dispatcher::new("serial", profile);
            let mut map = BTreeMap::new();
            let mut next = 0;
            loop {
                if let Some(frames) = recv.recv() {
                    if let Some(frames) = frames {
                        if !frames.is_empty() {
                            map.insert(frames[0].index() as usize, frames);
                        }
                        while let Some(mut frames) = map.remove(&next) {
                            next = frames.last().unwrap().index() as usize + 1;
                            callback.done(frames);
                        }
                    } else {
                        return;
                    }
                } else {
                    return;
                }
            }
        });
        handles.push(handle);

        Pool {
            sender: send,
            handles,
        }
    }

    pub fn process(&mut self, frames: Vec<Frame>) {
        self.sender.send(Some(frames));
    }
}

impl Drop for Pool {
    fn drop(&mut self) {
        for _ in 0..self.handles.len() {
            self.sender.send(None);
        }
        while let Some(h) = self.handles.pop() {
            h.join().unwrap();
        }
    }
}
