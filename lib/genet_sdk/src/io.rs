pub use genet_abi::io::{Reader, ReaderBox, Writer, WriterBox};

#[macro_export]
macro_rules! genet_readers {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static READERS: Vec<genet_sdk::io::ReaderBox> = {
                use genet_sdk::io::ReaderBox;
                let mut v = Vec::new();
                $(
                    v.push(ReaderBox::new($x));
                )*
                v
            };
        }
        #[no_mangle]
        pub extern "C" fn genet_abi_v1_get_readers(len: *mut u64) -> *const genet_sdk::io::ReaderBox {
            READERS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}

#[macro_export]
macro_rules! genet_writers {
    ( $( $x:expr ), * ) => {
        thread_local! {
            static WRITERS: Vec<genet_sdk::io::WriterBox> = {
                use genet_sdk::io::WriterBox;
                let mut v = Vec::new();
                $(
                    v.push(WriterBox::new($x));
                )*
                v
            };
        }
        #[no_mangle]
        pub extern "C" fn genet_abi_v1_get_writers(len: *mut u64) -> *const genet_sdk::io::WriterBox {
            WRITERS.with(|d| {
                unsafe {
                    *len = d.len() as u64;
                }
                d.as_ptr()
            })
        }
    };
}
