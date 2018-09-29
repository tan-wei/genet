//! Decoder traits.

pub use genet_abi::decoder::{Decoder, ExecType, Status, Worker};

#[doc(hidden)]
pub use genet_abi::decoder::DecoderBox;

/// Registers decoder entries.
#[macro_export]
macro_rules! genet_decoders {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static DISSECTORS: Vec<genet_sdk::decoder::DecoderBox> = {
                use genet_sdk::decoder::DecoderBox;
                let mut v = Vec::new();
                $(
                    v.push(DecoderBox::new($x));
                )*
                v
            };
        }
        #[no_mangle]
        pub extern "C" fn genet_abi_v1_get_decoders(len: *mut u64) -> *const genet_sdk::decoder::DecoderBox {
            DISSECTORS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
