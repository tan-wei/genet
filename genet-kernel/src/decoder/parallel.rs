use crossbeam_channel;
use decoder::{dispatcher::Dispatcher, Callback};
use frame::Frame;
use profile::Profile;
use std::thread::{self, JoinHandle};

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
            let mut disp = Dispatcher::new("parallel", &profile, &callback);
            loop {
                if let Some(frames) = recv.recv() {
                    if let Some(frames) = frames {
                        disp.process_frames(frames);
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
