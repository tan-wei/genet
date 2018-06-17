extern crate pcap;
extern crate clap;
extern crate serde_json;

use clap::{App, Arg, SubCommand};
use pcap::Pcap;
use std::io::Write;
use std::io::stdout;

fn main() {
	let capture = SubCommand::with_name("capture")
	                          .arg(Arg::with_name("DEVICE")
	                          .help("Sets the input file to use")
	                          .required(true)
	                          .index(1));

	let status = SubCommand::with_name("devices");

    let matches = App::new("pcap_cli")
    	.subcommand(capture)
    	.subcommand(status)
       .get_matches();

    let mut pcap = match Pcap::new() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("error: {:?}", e);
            ::std::process::exit(1)
        },
    };

    if let Some(_) = matches.subcommand_matches("devices") {
        let devices = pcap.devices().unwrap_or_else(|| vec![]);
        let _ = serde_json::to_writer(stdout(), &devices);
        println!();
    }

    if let Some(matches) = matches.subcommand_matches("capture") {
        let recv = match pcap.start(&matches.value_of("DEVICE").unwrap()) {
            Ok(r) => r,
            Err(e) => {
                eprintln!("error: {:?}", e);
                ::std::process::exit(1)
            },
        };

        while let Ok((header, data)) = recv.recv() {
            let _ = serde_json::to_writer(stdout(), &header);
            println!();
            let _ = stdout().write_all(&data);
        }
    }
}