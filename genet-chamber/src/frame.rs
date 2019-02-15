#![forbid(unsafe_code)]

use crate::attr::AttrTypeSet;

pub(crate) struct FrameData {
    pub data: Vec<u8>,
    pub attr_sets: Vec<*const AttrTypeSet>,
}
