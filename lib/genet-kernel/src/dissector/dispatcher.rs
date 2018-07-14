use frame::Frame;
use genet_abi::{
    context::Context,
    dissector::{DissectorBox, WorkerBox},
    layer::Layer,
    ptr::MutPtr,
    token::Token,
};
use profile::Profile;

pub struct Dispatcher {
    runners: Vec<Runner>,
}

impl Dispatcher {
    pub fn new(typ: &str, profile: Profile) -> Dispatcher {
        let mut runners = profile
            .dissectors()
            .map(|d| Runner::new(typ, profile.context(), d.clone()))
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
        let mut nextlayers = Vec::new();
        let mut runners = self.runners();
        loop {
            let mut indices = Vec::new();
            for mut child in sublayers.iter_mut() {
                let mut children = Vec::new();
                loop {
                    let mut executed = 0;
                    for mut r in &mut runners.iter_mut() {
                        let (done, mut layers) = r.execute(&children, child);
                        if done {
                            executed += 1;
                        }
                        children.append(&mut layers);
                    }
                    if executed == 0 {
                        break;
                    }
                }
                indices.push(children.len() as u8);
                nextlayers.append(&mut children);
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
    ) -> (bool, Vec<MutPtr<Layer>>) {
        let result = if let Some(worker) = &mut self.worker {
            let mut children = Vec::new();
            match worker.analyze(&mut self.ctx, layers, layer, &mut children) {
                Ok(done) => (done, children),
                Err(_) => (true, vec![]),
            }
        } else {
            (true, vec![])
        };
        result
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
    ) -> (bool, Vec<MutPtr<Layer>>) {
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
