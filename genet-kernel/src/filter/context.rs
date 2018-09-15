use frame::Frame;

pub struct Context<'a> {
    frame: &'a Frame,
}

impl<'a> Context<'a> {
    pub fn new(frame: &'a Frame) -> Self {
        Context { frame }
    }

    pub fn frame(&self) -> &'a Frame {
        self.frame
    }
}
