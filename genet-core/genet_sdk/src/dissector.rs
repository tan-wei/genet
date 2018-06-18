pub use genet_ffi::dissector::{Dissector, DissectorBox, Status, Worker};

#[macro_export]
macro_rules! genet_dissector {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static DISSECTORS: Vec<genet_sdk::dissector::DissectorBox> = {
                use genet_sdk::dissector::DissectorBox;
                let mut v = Vec::new();
                $(
                    v.push(DissectorBox::new($x));
                )*
                v
            };
        }
        #[no_mangle]
        pub extern "C" fn genet_ffi_v1_get_dissectors(len: *mut u64) -> *const genet_sdk::dissector::DissectorBox {
            DISSECTORS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
