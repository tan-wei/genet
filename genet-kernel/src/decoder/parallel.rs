use crossbeam_channel;
use decoder::dispatcher::Dispatcher;
use frame::Frame;
use genet_abi::decoder::ExecType;
use profile::Profile;
use std::thread::{self, JoinHandle};

pub trait Callback: Sync + Send + Clone {
    fn done(&self, result: Vec<Frame>);
}

pub struct Pool {
    sender: crossbeam_channel::Sender<Option<Vec<Frame>>>,
    handles: Vec<JoinHandle<()>>,
}

impl Pool {
    pub fn new<C: 'static + Callback>(profile: &Profile, callback: &C) -> Pool {
        let (send, recv) = crossbeam_channel::unbounded::<Option<Vec<Frame>>>();
        let mut handles = Vec::new();
        for _ in 0..profile.concurrency() {
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
        recv: crossbeam_channel::Receiver<Option<Vec<Frame>>>,
    ) -> JoinHandle<()> {
        thread::spawn(move || {
            let mut disp = Dispatcher::new(&ExecType::ParallelSync, &profile);
            loop {
                if let Some(frames) = recv.recv() {
                    if let Some(mut frames) = frames {
                        for mut f in &mut frames {
                            disp.process_frame(f);
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
