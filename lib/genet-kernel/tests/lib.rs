extern crate genet_abi;
extern crate genet_kernel;
extern crate libloading;
mod data;

#[macro_use]
extern crate genet_sdk;

use genet_kernel::{
    profile::Profile,
    session::{Callback, Event, Session},
};
use genet_sdk::{
    layer::{Layer, LayerClass},
    ptr::MutPtr,
};
use std::iter;

struct SessionCallback {}

impl Callback for SessionCallback {
    fn on_event(&self, event: Event) {
        if let Event::Frames(len) = event {
            println!("{:?}", len);
            if len == 40000 {
                ::std::process::exit(0);
            }
        }
    }
}

#[test]
fn session() {
    let mut profile = Profile::new();

    let libdir = ::std::env::current_exe()
        .unwrap()
        .parent()
        .unwrap()
        .to_path_buf()
        .join("../examples");

    profile
        .load_library(libdir.join("libeth.dylib").to_str().unwrap())
        .expect("failed to load dylib");

    profile
        .load_library(libdir.join("libreader.dylib").to_str().unwrap())
        .expect("failed to load dylib");

    let mut session = Session::new(profile, SessionCallback {});

    assert_eq!(session.create_reader("test-input", ""), 1);

    loop {}
}
