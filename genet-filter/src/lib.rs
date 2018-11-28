extern crate genet_abi;
extern crate hwaddr;
extern crate num_bigint;
extern crate num_traits;
extern crate pest;
extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate pest_derive;

#[macro_use]
extern crate arrayref;

use ast::Expr;
use genet_abi::filter::{LayerContext, LayerFilter};
use parser::parse;
use result::Result;
use std::fmt;
use variant::VariantExt;

pub mod ast;
pub mod parser;
pub mod result;
pub mod unparser;
pub mod variant;

#[derive(Clone, Debug)]
pub struct CompiledLayerFilter {
    expr: Expr,
}

impl CompiledLayerFilter {
    pub fn compile(filter: &str) -> Result<Self> {
        match parse(filter) {
            Ok(expr) => Ok(Self { expr }),
            Err(err) => Err(Box::new(Error(format!("{}", err)))),
        }
    }
}

impl LayerFilter for CompiledLayerFilter {
    fn test(&self, ctx: &LayerContext) -> bool {
        self.expr.eval(ctx).is_truthy()
    }
}

#[derive(Debug)]
struct Error(String);

impl std::error::Error for Error {
    fn description(&self) -> &str {
        &self.0
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}
