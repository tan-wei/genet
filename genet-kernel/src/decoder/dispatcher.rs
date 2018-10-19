use frame::Frame;
use genet_abi::{
    context::Context,
    decoder::{DecoderBox, ExecType, Metadata, WorkerBox},
    fixed::MutFixed,
    layer::{Layer, Parent},
};
use profile::Profile;

pub struct Dispatcher {
    runners: Vec<Runner>,
}

impl Dispatcher {
    pub fn new(typ: &ExecType, profile: &Profile) -> Dispatcher {
        let runners = profile
            .decoders()
            .map(|d| Runner::new(typ, profile.context(), *d))
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
        let mut indices = frame.fetch_tree_indices();
        let mut layers = frame.fetch_layers();
        let mut offset = 0;
        let mut runners = self.runners();
        loop {
            let len = layers.len() - offset;
            for index in offset..layers.len() {
                if let Some(n) = indices.get(index) {
                    if *n > 0 {
                        continue;
                    }
                }
                let mut children = 0;
                loop {
                    let mut executed = 0;
                    for mut r in &mut runners.iter_mut() {
                        let mut layer =
                            Parent::from_mut_ref(unsafe { &mut *layers[index].as_mut_ptr() });
                        let done = r.execute(&layers, &mut layer);
                        if done {
                            executed += 1;
                        }
                        let mut results: Vec<MutFixed<Layer>> = layer
                            .children()
                            .iter()
                            .map(|v| unsafe { MutFixed::from_ptr(*v) })
                            .collect();
                        children += results.len();
                        layers.append(&mut results);
                    }
                    if executed == 0 {
                        break;
                    }
                }
                indices.push(children as u8);
            }

            offset += len;
            if offset >= layers.len() {
                break;
            }
        }

        frame.set_layers(layers);
        frame.set_tree_indices(indices);
    }
}

struct Runner {
    ctx: Context,
    typ: ExecType,
    decoder: DecoderBox,
    metadata: Metadata,
    worker: Option<WorkerBox>,
}

impl Runner {
    fn new(typ: &ExecType, ctx: Context, decoder: DecoderBox) -> Runner {
        let mut runner = Runner {
            ctx,
            typ: typ.clone(),
            decoder,
            metadata: decoder.metadata(),
            worker: None,
        };
        runner.reset();
        runner
    }

    fn execute(&mut self, layers: &[MutFixed<Layer>], layer: &mut Parent) -> bool {
        if let Some(worker) = &mut self.worker {
            match worker.decode(&mut self.ctx, layers, layer) {
                Ok(done) => done,
                Err(_) => true,
            }
        } else {
            true
        }
    }

    fn reset(&mut self) {
        self.worker = if self.metadata.exec_type == self.typ {
            Some(self.decoder.new_worker(&self.ctx))
        } else {
            None
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

    fn execute(&mut self, layers: &[MutFixed<Layer>], layer: &mut Parent) -> bool {
        if !self.used {
            let done = self.runner.execute(layers, layer);
            if done {
                self.used = true;
            }
            done
        } else {
            false
        }
    }
}
