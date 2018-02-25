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
use plugkit::token::Token;
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

fn get_status_code(val: u16) -> Option<Token> {
    match val {
        100 => Some(token!("http.status.continue")),
        101 => Some(token!("http.status.switchingProtocols")),
        102 => Some(token!("http.status.processing")),
        200 => Some(token!("http.status.ok")),
        201 => Some(token!("http.status.created")),
        202 => Some(token!("http.status.accepted")),
        203 => Some(token!("http.status.nonAuthoritativeInformation")),
        204 => Some(token!("http.status.noContent")),
        205 => Some(token!("http.status.resetContent")),
        206 => Some(token!("http.status.partialContent")),
        207 => Some(token!("http.status.multiStatus")),
        208 => Some(token!("http.status.alreadyReported")),
        226 => Some(token!("http.status.imUsed")),
        300 => Some(token!("http.status.multipleChoices")),
        301 => Some(token!("http.status.movedPermanently")),
        302 => Some(token!("http.status.found")),
        303 => Some(token!("http.status.seeOther")),
        304 => Some(token!("http.status.notModified")),
        305 => Some(token!("http.status.useProxy")),
        307 => Some(token!("http.status.temporaryRedirect")),
        308 => Some(token!("http.status.permanentRedirect")),
        400 => Some(token!("http.status.badRequest")),
        401 => Some(token!("http.status.unauthorized")),
        402 => Some(token!("http.status.paymentRequired")),
        403 => Some(token!("http.status.forbidden")),
        404 => Some(token!("http.status.notFound")),
        405 => Some(token!("http.status.methodNotAllowed")),
        406 => Some(token!("http.status.notAcceptable")),
        407 => Some(token!("http.status.proxyAuthenticationRequired")),
        408 => Some(token!("http.status.requestTimeout")),
        409 => Some(token!("http.status.conflict")),
        410 => Some(token!("http.status.gone")),
        411 => Some(token!("http.status.lengthRequired")),
        412 => Some(token!("http.status.preconditionFailed")),
        413 => Some(token!("http.status.payloadTooLarge")),
        414 => Some(token!("http.status.uriTooLong")),
        415 => Some(token!("http.status.unsupportedMediaType")),
        416 => Some(token!("http.status.rangeNotSatisfiable")),
        417 => Some(token!("http.status.expectationFailed")),
        421 => Some(token!("http.status.misdirectedRequest")),
        422 => Some(token!("http.status.unprocessableEntity")),
        423 => Some(token!("http.status.locked")),
        424 => Some(token!("http.status.failedDependency")),
        426 => Some(token!("http.status.upgradeRequired")),
        428 => Some(token!("http.status.preconditionRequired")),
        429 => Some(token!("http.status.tooManyRequests")),
        431 => Some(token!("http.status.requestHeaderFieldsTooLarge")),
        451 => Some(token!("http.status.unavailableForLegalReasons")),
        500 => Some(token!("http.status.internalServerError")),
        501 => Some(token!("http.status.notImplemented")),
        502 => Some(token!("http.status.badGateway")),
        503 => Some(token!("http.status.serviceUnavailable")),
        504 => Some(token!("http.status.gatewayTimeout")),
        505 => Some(token!("http.status.httpVersionNotSupported")),
        506 => Some(token!("http.status.variantAlsoNegotiates")),
        507 => Some(token!("http.status.insufficientStorage")),
        508 => Some(token!("http.status.loopDetected")),
        510 => Some(token!("http.status.notExtended")),
        511 => Some(token!("http.status.networkAuthenticationRequired")),
        _ => None,
    }
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

        let parser;
        match self.status {
            Status::Request => parser = Rc::clone(&self.req_parser),
            Status::Response => parser = Rc::clone(&self.res_parser),
            _ => return Ok(())
        }

        let parser = &parser.borrow();
        let child = layer.add_layer(ctx, token!("http"));
        child.add_tag(ctx, token!("http"));

        {
            let version = parser.http_version();
            let attr = child.add_attr(ctx, token!("http.version"));
            attr.set(&(version.0 as f64 + (version.1 as f64) / 10.0));
        }

        if let Some(id) = get_status_code(parser.status_code()) {
            {
                let attr = child.add_attr(ctx, token!("http.status"));
                attr.set_typ(token!("@enum"));
                attr.set(&parser.status_code());
            }
            {
                let attr = child.add_attr(ctx, id);
                attr.set(&true);
                attr.set_typ(token!("@novalue"));
            }
        }

        if parser.should_keep_alive() {
            let attr = child.add_attr(ctx, token!("http.keepAlive"));
            attr.set_typ(token!("@novalue"));
            attr.set(&true);
        }

        if parser.is_upgrade() {
            let attr = child.add_attr(ctx, token!("http.upgrade"));
            attr.set_typ(token!("@novalue"));
            attr.set(&true);
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
