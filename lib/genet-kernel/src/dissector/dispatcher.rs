use frame::{Frame, WorkerMode};
use genet_abi::{
    context::Context,
    dissector::{DissectorBox, WorkerBox},
    layer::Layer,
    ptr::MutPtr,
};
use profile::Profile;

pub struct Dispatcher {
    runners: Vec<Runner>,
}

impl Dispatcher {
    pub fn new(typ: &str, profile: &Profile) -> Dispatcher {
        let runners = profile
            .dissectors()
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
        let mut mode = WorkerMode::None;
        loop {
            let len = sublayers.len() - offset;
            for index in offset..sublayers.len() {
                let mut layer = unsafe { &mut *sublayers[index].as_mut_ptr() };

                let mut children = 0;
                loop {
                    let mut executed = 0;
                    for mut r in &mut runners.iter_mut() {
                        let (done, mut layers) = r.execute(&sublayers, &mut layer, &mut mode);
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
        frame.set_worker(mode);
    }
}

struct Runner {
    ctx: Context,
    typ: String,
    dissector: DissectorBox,
    worker: Option<WorkerBox>,
}

impl Runner {
    fn new(typ: &str, ctx: Context, dissector: DissectorBox) -> Runner {
        let mut runner = Runner {
            ctx,
            typ: typ.to_string(),
            dissector,
            worker: None,
        };
        runner.reset();
        runner
    }

    fn execute(
        &mut self,
        layers: &[MutPtr<Layer>],
        layer: &mut Layer,
        mode: &mut WorkerMode,
    ) -> (bool, Vec<MutPtr<Layer>>) {
        if let Some(worker) = &mut self.worker {
            *mode = mode.add(WorkerMode::None);
            let mut children = Vec::new();
            match worker.analyze(&mut self.ctx, layers, layer, &mut children) {
                Ok(done) => (done, children),
                Err(_) => (true, vec![]),
            }
        } else {
            (true, vec![])
        }
    }

    fn reset(&mut self) {
        self.worker = self.dissector.new_worker(&self.typ, &self.ctx);
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
        layers: &[MutPtr<Layer>],
        layer: &mut Layer,
        mode: &mut WorkerMode,
    ) -> (bool, Vec<MutPtr<Layer>>) {
        if !self.used {
            let (done, children) = self.runner.execute(layers, layer, mode);
            if done {
                self.used = true;
            }
            (done, children)
        } else {
            (false, vec![])
        }
    }
}
