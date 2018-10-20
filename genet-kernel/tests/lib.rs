extern crate genet_abi;
extern crate genet_kernel;
extern crate libloading;

extern crate genet_sdk;

use genet_kernel::{
    profile::Profile,
    session::{Callback, Event, Session},
};

#[derive(Clone)]
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

    if cfg!(target_os = "macos") {
        profile
            .load_library(libdir.join("libeth.dylib").to_str().unwrap())
            .expect("failed to load dylib");
        profile
            .load_library(libdir.join("libreader.dylib").to_str().unwrap())
            .expect("failed to load dylib");
    } else if cfg!(target_os = "linux") {
        profile
            .load_library(libdir.join("libeth.so").to_str().unwrap())
            .expect("failed to load dylib");
        profile
            .load_library(libdir.join("libreader.so").to_str().unwrap())
            .expect("failed to load dylib");
    } else if cfg!(target_os = "windows") {
        profile
            .load_library(libdir.join("eth.dll").to_str().unwrap())
            .expect("failed to load dylib");
        profile
            .load_library(libdir.join("reader.dll").to_str().unwrap())
            .expect("failed to load dylib");
    }

    let mut session = Session::new(profile, SessionCallback {});

    assert_eq!(session.create_reader("app.genet.reader.test-input", ""), 1);
}
