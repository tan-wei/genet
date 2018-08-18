mod dispatcher;
pub mod parallel;
pub mod serial;

use frame::Frame;
use metadata::Metadata;
pub trait Callback: Sync + Send + Clone {
    fn on_frame_decoded(&self, _result: Vec<Frame>) {}
    fn on_metadata_added(&self, _result: Vec<(usize, Metadata)>) {}
}
