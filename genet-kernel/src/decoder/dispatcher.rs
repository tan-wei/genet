use decoder::Callback;
use frame::Frame;
use genet_abi::{
    context::Context,
    decoder::{DecoderBox, WorkerBox},
    fixed::MutFixed,
    layer::Layer,
};
use metadata::Metadata;
use profile::Profile;

pub struct Dispatcher<C> {
    runners: Vec<Runner>,
    callback: C,
}

impl<C: Callback> Dispatcher<C> {
    pub fn new(typ: &str, profile: &Profile, callback: &C) -> Dispatcher<C> {
        let runners = profile
            .decoders()
            .map(|d| Runner::new(typ, profile.context(), *d))
            .collect();
        Dispatcher {
            runners,
            callback: callback.clone(),
        }
    }

    pub fn runners(&mut self) -> Vec<OnceRunner> {
        self.runners
            .iter_mut()
            .map(|r| OnceRunner::new(r))
            .collect()
    }

    pub fn process_frames(&mut self, mut frames: Vec<Frame>) {
        for mut f in frames.iter_mut() {
            self.process_frame(f);
        }
        self.callback.on_frame_decoded(frames);
    }

    pub fn process_frame(&mut self, frame: &mut Frame) {
        let mut sublayers = Vec::new();
        sublayers.append(frame.layers_mut());
        let mut indices = Vec::new();
        let mut metadata = Vec::new();
        let mut offset = 0;

        {
            let mut runners = self.runners();
            loop {
                let len = sublayers.len() - offset;
                for index in offset..sublayers.len() {
                    let mut layer = unsafe { &mut *sublayers[index].as_mut_ptr() };

                    let mut children = 0;
                    loop {
                        let mut executed = 0;
                        for mut r in &mut runners.iter_mut() {
                            let (done, mut layers, mut meta) = r.execute(&sublayers, &mut layer);
                            if done {
                                executed += 1;
                            }
                            children += layers.len();
                            sublayers.append(&mut layers);
                            metadata.append(&mut meta);
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
        }

        frame.append_layers(&mut sublayers);
        frame.append_tree_indices(&mut indices);

        if !metadata.is_empty() {
            self.callback.on_metadata_added(metadata);
        }
    }
}

struct Runner {
    ctx: Context,
    typ: String,
    decoder: DecoderBox,
    worker: Option<WorkerBox>,
}

impl Runner {
    fn new(typ: &str, ctx: Context, decoder: DecoderBox) -> Runner {
        let mut runner = Runner {
            ctx,
            typ: typ.to_string(),
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
    ) -> (bool, Vec<MutFixed<Layer>>, Vec<(usize, Metadata)>) {
        if let Some(worker) = &mut self.worker {
            let mut children = Vec::new();
            match worker.decode(&mut self.ctx, layers, layer, &mut children) {
                Ok(done) => (done, children, vec![]),
                Err(_) => (true, vec![], vec![]),
            }
        } else {
            (true, vec![], vec![])
        }
    }

    fn reset(&mut self) {
        self.worker = self.decoder.new_worker(&self.typ, &self.ctx);
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
    ) -> (bool, Vec<MutFixed<Layer>>, Vec<(usize, Metadata)>) {
        if !self.used {
            let (done, children, meta) = self.runner.execute(layers, layer);
            if done {
                self.used = true;
            }
            (done, children, meta)
        } else {
            (false, vec![], vec![])
        }
    }
}
