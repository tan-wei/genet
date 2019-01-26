use std::alloc::{GlobalAlloc, Layout};

static mut ALLOC: extern "C" fn(size: u64) -> *mut u8 = alloc;
static mut REALLOC: extern "C" fn(ptr: *mut u8, size: u64) -> *mut u8 = realloc;
static mut DEALLOC: extern "C" fn(ptr: *mut u8) = dealloc;

#[repr(C)]
pub struct Allocator {
    alloc: extern "C" fn(size: u64) -> *mut u8,
    realloc: extern "C" fn(ptr: *mut u8, size: u64) -> *mut u8,
    dealloc: extern "C" fn(ptr: *mut u8),
}

impl Default for Allocator {
    fn default() -> Self {
        Self {
            alloc,
            realloc,
            dealloc,
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_abi_v1_register_allocator(alloc: Allocator) {
    ALLOC = alloc.alloc;
    REALLOC = alloc.realloc;
    DEALLOC = alloc.dealloc;
}

extern "C" fn alloc(size: u64) -> *mut u8 {
    unsafe { libc::malloc(size as usize) as *mut u8 }
}
extern "C" fn realloc(ptr: *mut u8, size: u64) -> *mut u8 {
    unsafe { libc::realloc(ptr as *mut libc::c_void, size as usize) as *mut u8 }
}
extern "C" fn dealloc(ptr: *mut u8) {
    unsafe { libc::free(ptr as *mut libc::c_void) };
}

pub struct SharedAllocator;

unsafe impl GlobalAlloc for SharedAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        ALLOC(layout.size() as u64)
    }

    unsafe fn realloc(&self, ptr: *mut u8, _layout: Layout, new_size: usize) -> *mut u8 {
        REALLOC(ptr, new_size as u64)
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        DEALLOC(ptr)
    }
}
