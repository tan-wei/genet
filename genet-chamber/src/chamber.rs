use parking_lot::{Mutex, MutexGuard};
use std::sync::Arc;

pub struct Chamber {
    inner: Arc<Mutex<Inner>>,
}

impl Chamber {
    pub fn new() -> Self {
        Self {
            inner: Arc::new(Mutex::new(Inner {})),
        }
    }
}

struct Inner {}

pub struct ChamberScope;

impl ChamberScope {
    pub unsafe fn try_lock(&self, ptr: *mut Chamber) -> Option<ChamberLock> {
        (*ptr).inner.try_lock().map(|lock| ChamberLock { lock })
    }
}

pub struct ChamberLock<'a> {
    lock: MutexGuard<'a, Inner>,
}
