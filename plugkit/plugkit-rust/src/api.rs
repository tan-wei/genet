#[macro_export]
macro_rules! plugkit_api_layer_hints {
    ( $( $x:expr ), * ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_layer_hints(index: u32) -> u32 {
            let mut temp_vec = Vec::new();
            $(
                temp_vec.push($x);
            )*
            temp_vec.push(0);
            temp_vec[index as usize]
        }
    };
}

#[macro_export]
macro_rules! plugkit_api_worker {
    ( $x: ty, $y:expr ) => {
        #[no_mangle]
        pub extern "C" fn plugkit_v1_create_worker(_ctx: *const (), _diss: *const ()) -> *mut() {
            Box::into_raw(Box::new($y)) as *mut()
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_destroy_worker(_ctx: *const (), _diss: *const (), worker: *mut()) {
            unsafe {
                Box::from_raw(worker as *mut $x);
            }
        }

        #[no_mangle]
        pub extern "C" fn plugkit_v1_analyze(ctx: *mut (), _diss: *const (), worker: *mut(), layer: *mut ()) {
            unsafe {
                let w: &$x = &*(worker as *mut $x);
                let c: &mut ::plugkit::context::Context = &mut *(ctx as *mut ::plugkit::context::Context);
                let l: &mut ::plugkit::layer::Layer = &mut *(layer as *mut ::plugkit::layer::Layer);
                let result = ::std::panic::catch_unwind(::std::panic::AssertUnwindSafe(|| {
                    w.analyze(c, l)
                }));
                match result {
                    Ok(_) => {},
                    Err(e) => {
                        println!("{:?}", e);
                    },
                };
            }
        }
    };
}
