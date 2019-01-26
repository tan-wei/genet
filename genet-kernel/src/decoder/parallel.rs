use crate::{frame::Frame, profile::Profile};
use crossbeam_channel;
use genet_abi::layer::{LayerStack, LayerStackData};
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
            if let Some(mut decoder) = profile.context().decoder("app.genet.decoder.pcap_layer") {
                loop {
                    if let Ok(frames) = recv.recv() {
                        if let Some(mut frames) = frames {
                            for f in &mut frames {
                                let mut layers = f.fetch_layers();

                                let mut data = LayerStackData {
                                    children: Vec::new(),
                                };
                                let mut layer = LayerStack::from_mut_ref(&mut data, unsafe {
                                    &mut *layers[0].as_mut_ptr()
                                });
                                let _ = decoder.decode(&mut layer);
                                layers.append(&mut data.children);
                                f.set_layers(layers);
                            }
                            callback.done(frames);
                        } else {
                            return;
                        }
                    } else {
                        return;
                    }
                }
            }
        })
    }

    pub fn process(&mut self, frames: Vec<Frame>) {
        let _ = self.sender.send(Some(frames));
    }
}

impl Drop for Pool {
    fn drop(&mut self) {
        for _ in 0..self.handles.len() {
            let _ = self.sender.send(None);
        }
        while let Some(h) = self.handles.pop() {
            h.join().unwrap();
        }
    }
}
