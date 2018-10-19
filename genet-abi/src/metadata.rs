use std::{slice, str};

#[repr(C)]
pub struct Metadata {
    name: *const u8,
    description: *const u8,
    name_len: u16,
    description_len: u16,
}

unsafe impl Send for Metadata {}
unsafe impl Sync for Metadata {}

impl Default for Metadata {
    fn default() -> Self {
        Self::new()
    }
}

impl Metadata {
    pub fn new() -> Metadata {
        Metadata {
            name: "".as_ptr(),
            description: "".as_ptr(),
            name_len: 0,
            description_len: 0,
        }
    }

    pub fn name(&self) -> &'static str {
        unsafe {
            str::from_utf8_unchecked(slice::from_raw_parts(self.name, self.name_len as usize))
        }
    }

    pub fn description(&self) -> &'static str {
        unsafe {
            str::from_utf8_unchecked(slice::from_raw_parts(
                self.description,
                self.description_len as usize,
            ))
        }
    }

    pub fn set_name(&mut self, name: &'static str) {
        self.name = name.as_ptr();
        self.name_len = name.len() as u16;
    }

    pub fn set_description(&mut self, desc: &'static str) {
        self.description = desc.as_ptr();
        self.description_len = desc.len() as u16;
    }
}
