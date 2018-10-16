use filter::{ast::Expr, context::Context, parser::parse, variant::VariantExt};
use result::Result;
use std::fmt;

pub mod ast;
pub mod context;
pub mod parser;
pub mod variant;

#[derive(Clone, Debug)]
pub struct Filter {
    expr: Expr,
}

impl Filter {
    pub fn compile(filter: &str) -> Result<Filter> {
        match parse(filter) {
            Ok(expr) => Ok(Filter { expr }),
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
