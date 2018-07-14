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
