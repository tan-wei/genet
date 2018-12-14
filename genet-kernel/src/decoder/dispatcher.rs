use crate::{frame::Frame, profile::Profile};
use genet_abi::{
    decoder::{ExecType, WorkerBox},
    layer::{LayerStack, LayerStackData},
};

pub struct Dispatcher {
    runners: Vec<Runner>,
}

impl Dispatcher {
    pub fn new(typ: &ExecType, profile: &Profile) -> Dispatcher {
        let runners = profile
            .decoders()
            .filter(|d| d.metadata().exec_type == *typ)
            .map(|d| Runner::new(d.new_worker(&profile.context())))
            .collect();
        Dispatcher { runners }
    }

    pub fn runners(&mut self) -> Vec<OnceRunner> {
        self.runners
            .iter_mut()
            .map(|r| OnceRunner::new(r))
            .collect()
    }

    pub fn process_frame(&mut self, frame: &mut Frame) {
        let mut layers = frame.fetch_layers();
        let mut offset = 0;
        let mut runners = self.runners();
        loop {
            let len = layers.len() - offset;
            for index in offset..layers.len() {
                loop {
                    let mut executed = 0;
                    for r in &mut runners.iter_mut() {
                        let mut data = LayerStackData {
                            children: Vec::new(),
                        };
                        let mut layer = LayerStack::from_mut_ref(&mut data, unsafe {
                            &mut *layers[index].as_mut_ptr()
                        });
                        let done = r.execute(&mut layer);
                        if done {
                            executed += 1;
                        }
                        layers.append(&mut data.children);
                    }
                    if executed == 0 {
                        break;
                    }
                }
            }

            offset += len;
            if offset >= layers.len() {
                break;
            }
        }

        frame.set_layers(layers);
    }
}

struct Runner {
    worker: WorkerBox,
}

impl Runner {
    fn new(worker: WorkerBox) -> Runner {
        Runner { worker }
    }

    fn execute(&mut self, layer: &mut LayerStack) -> bool {
        match self.worker.decode(layer) {
            Ok(done) => done,
            Err(_) => true,
        }
    }
}

pub struct OnceRunner<'a> {
    runner: &'a mut Runner,
    used: bool,
}

impl<'a> OnceRunner<'a> {
    fn new(runner: &'a mut Runner) -> OnceRunner {
        OnceRunner {
            runner,
            used: false,
        }
    }

    fn execute(&mut self, layer: &mut LayerStack) -> bool {
        if !self.used {
            let done = self.runner.execute(layer);
            if done {
                self.used = true;
            }
            done
        } else {
            false
        }
    }
}
