use chan;
use dissector::dispatcher::Dispatcher;
use frame::Frame;
use profile::Profile;
use std::thread::{self, JoinHandle};

pub trait Callback: Sync + Send + Clone {
    fn done(&self, result: Vec<Frame>);
}

pub struct Pool {
    sender: chan::Sender<Option<Vec<Frame>>>,
    handles: Vec<JoinHandle<()>>,
}

impl Pool {
    pub fn new<C: 'static + Callback>(profile: Profile, callback: C) -> Pool {
        let (send, recv) = chan::async::<Option<Vec<Frame>>>();
        let mut handles = Vec::new();
        for i in 0..profile.concurrency() {
            handles.push(Self::spawn(profile.clone(), callback.clone(), recv.clone()));
        }
        Pool {
            sender: send,
            handles,
        }
    }

    fn spawn<C: 'static + Callback>(
        profile: Profile,
        callback: C,
        recv: chan::Receiver<Option<Vec<Frame>>>,
    ) -> JoinHandle<()> {
        thread::spawn(move || {
            let mut disp = Dispatcher::new("parallel", profile);
            loop {
                if let Some(frames) = recv.recv() {
                    if let Some(mut frames) = frames {
                        for mut f in frames.iter_mut() {
                            Self::process_frame(&mut disp, f);
                        }
                        callback.done(frames);
                    } else {
                        return;
                    }
                } else {
                    return;
                }
            }
        })
    }

    fn process_frame(disp: &mut Dispatcher, frame: &mut Frame) {
        let mut sublayers = Vec::new();
        sublayers.append(frame.layers_mut());
        let mut nextlayers = Vec::new();
        let mut runners = disp.runners();
        loop {
            let mut indices = Vec::new();
            for mut child in sublayers.iter_mut() {
                let mut results = Dispatcher::execute(&mut runners, &mut child);
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
