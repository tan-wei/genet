pub use dyr_ffi::dissector::{Dissector, DissectorBox, Status, Worker};

#[macro_export]
macro_rules! dyr_dissector {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static DISSECTORS: Vec<dyr_sdk::dissector::DissectorBox> = {
                use dyr_sdk::dissector::DissectorBox;
                let mut v = Vec::new();
                $(
                    v.push(DissectorBox::new($x));
                )*
                v
            };
        }
        #[no_mangle]
        pub extern "C" fn dyr_ffi_v1_get_dissectors(len: *mut u64) -> *const dyr_sdk::dissector::DissectorBox {
            DISSECTORS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
