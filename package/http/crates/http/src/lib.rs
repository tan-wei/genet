extern crate http_muncher;
extern crate byteorder;
extern crate libc;

#[macro_use]
extern crate plugkit;

use std::collections::HashMap;
use std::io::{Error, ErrorKind};
use plugkit::layer::{Layer};
use plugkit::context::Context;
use plugkit::worker::Worker;
use plugkit::variant::Value;
use http_muncher::{Parser, ParserHandler};
use std::cell::RefCell;
use std::rc::Rc;

enum Status {
    None,
    Request,
    Response,
    Error
}

struct HTTPSession {
    status: Status,
    req_parser: Rc<RefCell<Parser>>,
    res_parser: Rc<RefCell<Parser>>
}

impl ParserHandler for HTTPSession {}

impl HTTPSession {
    pub fn new() -> HTTPSession {
        HTTPSession {
            status: Status::None,
            req_parser: Rc::new(RefCell::new(Parser::request())),
            res_parser: Rc::new(RefCell::new(Parser::response()))
        }
    }

    fn parse_request(&mut self, slice: &'static[u8]) -> usize {
        let parser = Rc::clone(&self.req_parser);
        let size = parser.borrow_mut().parse(self, &slice);
        size
    }

    fn parse_response(&mut self, slice: &'static[u8]) -> usize {
        let parser = Rc::clone(&self.res_parser);
        let size = parser.borrow_mut().parse(self, &slice);
        size
    }

    fn analyze(&mut self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
        let (slice, _) = {
            let payload = layer
                .payloads()
                .next()
                .ok_or(Error::new(ErrorKind::Other, "no payload"))?;
            let slice = payload
                .slices()
                .next()
                .ok_or(Error::new(ErrorKind::Other, "no slice"))?;
            (slice, payload.range())
        };

        match self.status {
            Status::None => {
                self.status = if slice.len() == 0 {
                    Status::None
                } else if self.parse_request(slice) == slice.len() {
                    Status::Request
                } else if self.parse_response(slice) == slice.len() {
                    Status::Response
                } else {
                    Status::Error
                }
            },
            Status::Request => {
                if self.parse_request(slice) != slice.len() {
                    self.status = Status::Error;
                }
            },
            Status::Response => {
                if self.parse_response(slice) != slice.len() {
                    self.status = Status::Error;
                }
            }
            _ => ()
        }

        match self.status {
            Status::Request | Status::Response => {
                let child = layer.add_layer(ctx, token!("http"));
                child.add_tag(ctx, token!("http"));
            }
            Status::Error => ctx.close_stream(),
            _ => ()
        }

        Ok(())
    }
}

struct HTTPWorker {
    map: HashMap<u32, HTTPSession>
}

impl HTTPWorker {
    pub fn new() -> HTTPWorker {
        HTTPWorker {
            map: HashMap::new()
        }
    }
}

impl Worker for HTTPWorker {
    fn analyze(&mut self, ctx: &mut Context, layer: &mut Layer) -> Result<(), Error> {
        let stream_id: u32 = {
            layer.attr(token!("_.streamId")).unwrap().get()
        };
        let session = self.map.entry(stream_id).or_insert_with(|| HTTPSession::new());
        session.analyze(ctx, layer)
    }
}

plugkit_module!({});
plugkit_api_layer_hints!(token!("tcp-stream"));
plugkit_api_worker!(HTTPWorker, HTTPWorker::new());
