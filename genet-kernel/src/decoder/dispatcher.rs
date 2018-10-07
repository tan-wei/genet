use frame::Frame;
use genet_abi::{
    context::Context,
    decoder::{DecoderBox, ExecType, WorkerBox},
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
        let mut sublayers: Vec<*mut Layer> = frame
            .layers()
            .map(|v| v.as_mut_ptr())
            .collect();
        let mut offset = 0;
        let mut runners = self.runners();
        loop {
            let len = sublayers.len() - offset;
            for index in offset..sublayers.len() {
                let mut layer = Parent::from_ref(unsafe { &mut *sublayers[index] });
                loop {
                    let mut executed = false;
                    for mut r in &mut runners.iter_mut() {
                        executed = executed || r.execute(&sublayers, &mut layer);
                    }
                    if !executed {
                        break;
                    }
                }
                let mut layers = layer
                    .children()
                    .iter()
                    .map(|v| v.as_mut_ptr())
                    .collect::<Vec<_>>();
                sublayers.append(&mut layers);
                layer.apply();
            }

            offset += len;
            if offset >= sublayers.len() {
                break;
            }
        }
    }
}

struct Runner {
    ctx: Context,
    typ: ExecType,
    decoder: DecoderBox,
    worker: Option<WorkerBox>,
}

impl Runner {
    fn new(typ: &ExecType, ctx: Context, decoder: DecoderBox) -> Runner {
        let mut runner = Runner {
            ctx,
            typ: typ.clone(),
            decoder,
            worker: None,
        };
        runner.reset();
        runner
    }

    fn execute(&mut self, layers: &[*mut Layer], layer: &mut Parent) -> bool {
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
        self.worker = if self.decoder.execution_type() == self.typ {
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

    fn execute(&mut self, layers: &[*mut Layer], layer: &mut Parent) -> bool {
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
