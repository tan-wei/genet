#![windows_subsystem = "windows"]

extern crate clap;
extern crate pcap;
extern crate serde_json;

use clap::{App, Arg, SubCommand};
use pcap::Pcap;
use std::{
    io::{stdout, Write},
    sync::mpsc::RecvTimeoutError,
    time::Duration,
};

fn main() {
    let capture = SubCommand::with_name("capture")
        .arg(
            Arg::with_name("DEVICE")
                .help("Sets the input file to use")
                .required(true)
                .index(1),
        )
        .arg(
            Arg::with_name("snaplen")
                .short("l")
                .help("Sets the snapshot length")
                .takes_value(true),
        );

    let status = SubCommand::with_name("devices");

    let matches = App::new("pcap-cli")
        .arg(
            Arg::with_name("timeout")
                .takes_value(true)
                .default_value("1000")
                .short("t"),
        )
        .subcommand(capture)
        .subcommand(status)
        .get_matches();

    let timeout: u64 = matches.value_of("timeout").unwrap().parse().unwrap_or(1000);
    let timeout = Duration::from_millis(timeout);

    let mut pcap = match Pcap::new() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("error: {:?}", e);
            ::std::process::exit(1)
        }
    };

    if matches.subcommand_matches("devices").is_some() {
        let devices = pcap.devices().unwrap_or_else(|| vec![]);
        let _ = serde_json::to_writer(stdout(), &devices);
        println!();
    }

    if let Some(matches) = matches.subcommand_matches("capture") {
        let snaplen: u32 = matches
            .value_of("snaplen")
            .and_then(|v| v.parse().ok())
            .unwrap_or(2048);
        let recv = match pcap.start(&matches.value_of("DEVICE").unwrap(), snaplen) {
            Ok(r) => r,
            Err(e) => {
                eprintln!("error: {:?}", e);
                ::std::process::exit(1)
            }
        };

        loop {
            match recv.recv_timeout(timeout) {
                Ok((header, data)) => {
                    let _ = serde_json::to_writer(stdout(), &header);
                    println!();
                    let _ = stdout().write_all(&data);
                }
                Err(RecvTimeoutError::Disconnected) => return,
                _ => println!(),
            }
        }
    }
}
