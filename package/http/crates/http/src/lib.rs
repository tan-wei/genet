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
use plugkit::variant;
use plugkit::variant::{Value};
use http_muncher::{Parser, ParserHandler, ParserType, Method};
use std::cell::RefCell;
use std::str;
use std::rc::Rc;

#[derive(PartialEq)]
enum Status {
    None,
    Active,
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

fn get_method(val: Method) -> Token {
    match val {
        Method::HttpDelete => token!("http.method.delete"),
        Method::HttpGet => token!("http.method.get"),
        Method::HttpHead => token!("http.method.head"),
        Method::HttpPost => token!("http.method.post"),
        Method::HttpPut => token!("http.method.put"),
        Method::HttpConnect => token!("http.method.connect"),
        Method::HttpOptions => token!("http.method.options"),
        Method::HttpTrace => token!("http.method.trace"),
        Method::HttpCopy => token!("http.method.copy"),
        Method::HttpLock => token!("http.method.lock"),
        Method::HttpMkcol => token!("http.method.mkcol"),
        Method::HttpMove => token!("http.method.move"),
        Method::HttpPropfind => token!("http.method.propfind"),
        Method::HttpProppatch => token!("http.method.proppatch"),
        Method::HttpSearch => token!("http.method.search"),
        Method::HttpUnlock => token!("http.method.unlock"),
        Method::HttpBind => token!("http.method.bind"),
        Method::HttpRebind => token!("http.method.rebind"),
        Method::HttpUnbind => token!("http.method.unbind"),
        Method::HttpAcl => token!("http.method.acl"),
        Method::HttpReport => token!("http.method.report"),
        Method::HttpMkactivity => token!("http.method.mkactivity"),
        Method::HttpCheckout => token!("http.method.checkout"),
        Method::HttpMerge => token!("http.method.merge"),
        Method::HttpMSearch => token!("http.method.mSearch"),
        Method::HttpNotify => token!("http.method.notify"),
        Method::HttpSubscribe => token!("http.method.subscribe"),
        Method::HttpUnsubscribe => token!("http.method.unsubscribe"),
        Method::HttpPatch => token!("http.method.patch"),
        Method::HttpPurge => token!("http.method.purge"),
        Method::HttpMkcalendar => token!("http.method.mkcalendar"),
        Method::HttpLink => token!("http.method.link"),
        Method::HttpUnlink => token!("http.method.unlink"),
        Method::HttpSource => token!("http.method.source"),
    }
}

enum Entry {
    Url(&'static[u8])
}

struct HTTPSession {
    status: Status,
    parser: Rc<RefCell<Parser>>,
    entries: Vec<Entry>
}

impl ParserHandler for HTTPSession {
    fn on_url(&mut self, _parser: &mut Parser, data: &'static[u8]) -> bool {
        self.entries.push(Entry::Url(data));
        return true
    }
}

impl HTTPSession {
    pub fn new() -> HTTPSession {
        HTTPSession {
            status: Status::None,
            parser: Rc::new(RefCell::new(Parser::request_and_response())),
            entries: Vec::new()
        }
    }

    fn parse(&mut self, slice: &'static[u8]) -> usize {
        let parser = Rc::clone(&self.parser);
        let size = parser.borrow_mut().parse(self, &slice);
        size
    }

    fn analyze(&mut self, ctx: &mut Context, layer: &mut Layer, stream_id: u64) -> Result<(), Error> {
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
                } else if self.parse(slice) == slice.len() {
                    Status::Active
                } else {
                    Status::Error
                }
            },
            Status::Active => {
                if self.parse(slice) != slice.len() {
                    self.status = Status::Error;
                }
            },
            _ => ()
        }

        if self.status != Status::Active {
            return Ok(())
        }

        let parser = Rc::clone(&self.parser);
        let parser = &parser.borrow();

        let child = layer.add_layer(ctx, token!("http"));
        child.add_tag(ctx, token!("http"));
        ctx.add_layer_linkage(stream_id, child);

        {
            let version = parser.http_version();
            let attr = child.add_attr(ctx, token!("http.version"));
            attr.set(&(version.0 as f64 + (version.1 as f64) / 10.0));
        }

        if parser.parser_type() == ParserType::HttpRequest {
            {
                let attr = child.add_attr(ctx, token!("http.method"));
                variant::ValueString::set(attr, parser.http_method_str());
                attr.set_typ(token!("@enum"));
            }
            {
                let attr = child.add_attr(ctx, get_method(parser.http_method()));
                attr.set(&true);
                attr.set_typ(token!("@novalue"));
            }
        }

        if parser.parser_type() == ParserType::HttpResponse {
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
        }

        if parser.should_keep_alive() {
            let attr = child.add_attr(ctx, token!("http.keepAlive"));
            attr.set_typ(token!("@novalue"));
            attr.set(&true);
        }

        while let Some(top) = self.entries.pop() {
            match top {
                Entry::Url(data) => {
                    if let Ok(path) = str::from_utf8(data) {
                        let attr = child.add_attr(ctx, token!("http.path"));
                        variant::ValueString::set(attr, &path);
                    }
                }
            }
        }

        Ok(())
    }
}

struct HTTPWorker {
    map: HashMap<u64, HTTPSession>
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
        let stream_id: u64 = {
            layer.attr(token!("_.streamId")).unwrap().get()
        };
        let session = self.map.entry(stream_id).or_insert_with(|| HTTPSession::new());
        session.analyze(ctx, layer, stream_id)
    }
}

plugkit_module!({});
plugkit_api_layer_hints!(token!("tcp-stream"));
plugkit_api_worker!(HTTPWorker, HTTPWorker::new());
