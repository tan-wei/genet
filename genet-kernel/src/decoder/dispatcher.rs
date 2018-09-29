use frame::Frame;
use genet_abi::{
    context::Context,
    decoder::{DecoderBox, ExecType, WorkerBox},
    fixed::MutFixed,
    layer::Layer,
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
        let mut sublayers = Vec::new();
        sublayers.append(frame.layers_mut());
        let mut indices = Vec::new();
        let mut offset = 0;
        let mut runners = self.runners();
        loop {
            let len = sublayers.len() - offset;
            for index in offset..sublayers.len() {
                let mut layer = unsafe { &mut *sublayers[index].as_mut_ptr() };

                let mut children = 0;
                loop {
                    let mut executed = 0;
                    for mut r in &mut runners.iter_mut() {
                        let (done, mut layers) = r.execute(&sublayers, &mut layer);
                        if done {
                            executed += 1;
                        }
                        children += layers.len();
                        sublayers.append(&mut layers);
                    }
                    if executed == 0 {
                        break;
                    }
                }
                indices.push(children as u8);
            }

            offset += len;
            if offset >= sublayers.len() {
                break;
            }
        }

        frame.append_layers(&mut sublayers);
        frame.append_tree_indices(&mut indices);
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

    fn execute(
        &mut self,
        layers: &[MutFixed<Layer>],
        layer: &mut Layer,
    ) -> (bool, Vec<MutFixed<Layer>>) {
        if let Some(worker) = &mut self.worker {
            let mut children = Vec::new();
            match worker.decode(&mut self.ctx, layers, layer, &mut children) {
                Ok(done) => (done, children),
                Err(_) => (true, vec![]),
            }
        } else {
            (true, vec![])
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

    fn execute(
        &mut self,
        layers: &[MutFixed<Layer>],
        layer: &mut Layer,
    ) -> (bool, Vec<MutFixed<Layer>>) {
        if !self.used {
            let (done, children) = self.runner.execute(layers, layer);
            if done {
                self.used = true;
            }
            (done, children)
        } else {
            (false, vec![])
        }
    }
}
