use combine::Parser;
use filter::{ast::Expr, context::Context, parser::expression};
use frame::Frame;
use result::Result;
use std::fmt::{self, Debug};

pub mod ast;
pub mod context;
pub mod parser;
pub mod variant;

pub trait Worker {
    fn test(&self, &Frame) -> bool {
        true
    }
}

pub trait Filter: Send + Debug {
    fn new_worker(&self) -> Box<Worker>;
}

#[derive(Clone, Debug)]
pub struct XFilter {
    expr: Expr,
}

impl XFilter {
    pub fn compile(filter: &str) -> Result<XFilter> {
        match expression().parse(filter) {
            Ok((expr, _)) => Ok(XFilter { expr }),
            Err(err) => Err(Box::new(Error(format!("{}", err)))),
        }
    }

    pub fn test(&self, ctx: &Context) -> bool {
        self.expr.eval(ctx).is_truthy()
    }
}

#[derive(Debug)]
struct Error(String);

impl ::std::error::Error for Error {
    fn description(&self) -> &str {
        &self.0
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}
