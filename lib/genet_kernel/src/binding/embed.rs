use libc;

macro_rules! embed {
    ($($arg:tt),*) => {
        #[no_mangle]
        pub extern "C" fn genet_embedded_js() -> *const libc::c_char {
            concat!(
                "(function(exports){\n",
                $(
                    "(function(){\n",
                    include_str!($arg),
                    "\n})();\n",
                )*
                "})\0"
            ).as_ptr() as *const libc::c_char
        }
    }
}

embed!();
