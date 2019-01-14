use crate::{ast::Expr, parser::parse, result::Result, variant::VariantExt};
use genet_abi::filter::{LayerContext, LayerFilter};

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
            Err(err) => Err(err.into()),
        }
    }
}

impl LayerFilter for CompiledLayerFilter {
    fn test(&self, ctx: &LayerContext) -> bool {
        self.expr.eval(ctx).is_truthy()
    }
}
