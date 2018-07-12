#[macro_use]
extern crate genet_sdk;

#[macro_use]
extern crate lazy_static;

extern crate http_muncher;

use genet_sdk::prelude::*;
use http_muncher::{Method, Parser, ParserHandler, ParserType};
use std::collections::HashMap;

struct HttpWorker {}

impl Worker for HttpWorker {
    fn analyze(&mut self, _ctx: &mut Context, parent: &mut Layer) -> Result<Status> {
        if parent.id() == token!("tcp") {
            let mut layer = Layer::new(&HTTP_CLASS, parent.data());
            Ok(Status::Done(vec![layer]))
        } else {
            Ok(Status::Skip)
        }
    }
}

#[derive(Clone)]
struct HttpDissector {}

impl Dissector for HttpDissector {
    fn new_worker(&self, typ: &str, _ctx: &Context) -> Option<Box<Worker>> {
        if typ == "serial" {
            Some(Box::new(HttpWorker {}))
        } else {
            None
        }
    }
}

lazy_static! {
    static ref HTTP_CLASS: Ptr<LayerClass> = LayerBuilder::new("http").build();
}

genet_dissectors!(HttpDissector {});

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
