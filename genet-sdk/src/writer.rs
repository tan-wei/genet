//! Writer traits.

pub use genet_abi::writer::{Metadata, Worker, Writer};

#[doc(hidden)]
pub use genet_abi::writer::WriterBox;

/// Registers writer entries.
#[macro_export]
macro_rules! genet_writers {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static WRITERS: Vec<genet_sdk::writer::WriterBox> = {
                use genet_sdk::writer::WriterBox;
                let mut v = Vec::new();
                $(
                    v.push(WriterBox::new($x));
                )*
                v
            };
        }
        #[cfg(not(feature = "genet-static"))]
        #[no_mangle]
        pub extern "C" fn genet_abi_v1_get_writers(len: *mut u64) -> *const genet_sdk::writer::WriterBox {
            WRITERS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
