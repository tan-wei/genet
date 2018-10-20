//! Reader traits.

pub use genet_abi::reader::{Reader, Worker};

#[doc(hidden)]
pub use genet_abi::reader::ReaderBox;

/// Registers reader entries.
#[macro_export]
macro_rules! genet_readers {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static READERS: Vec<genet_sdk::reader::ReaderBox> = {
                use genet_sdk::reader::ReaderBox;
                let mut v = Vec::new();
                $(
                    v.push(ReaderBox::new($x));
                )*
                v
            };
        }
        #[cfg(not(feature = "genet-static"))]
        #[no_mangle]
        pub extern "C" fn genet_abi_v1_get_readers(len: *mut u64) -> *const genet_sdk::reader::ReaderBox {
            READERS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
