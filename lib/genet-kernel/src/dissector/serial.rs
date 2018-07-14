use chan;
use dissector::dispatcher::Dispatcher;
use frame::Frame;
use genet_abi::layer::Layer;
use profile::Profile;
use std::{
    collections::BTreeMap,
    thread::{self, JoinHandle},
};

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
                            for frame in frames.iter_mut() {
                                Self::process_frame(&mut disp, frame);
                            }
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

    fn process_frame(disp: &mut Dispatcher, frame: &mut Frame) {
        let mut sublayers = Vec::new();
        sublayers.append(frame.layers_mut());
        let mut nextlayers = Vec::new();
        let mut runners = disp.runners();
        loop {
            let mut indices = Vec::new();
            for mut child in sublayers.iter_mut() {
                let mut results = Dispatcher::execute(&mut runners, frame, &mut child);
                indices.push(results.len() as u8);
                nextlayers.append(&mut results);
            }
            frame.append_layers(&mut sublayers);
            frame.append_tree_indices(&mut indices);
            if nextlayers.is_empty() {
                break;
            } else {
                sublayers.append(&mut nextlayers);
            }
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
