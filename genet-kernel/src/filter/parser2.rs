
use pest::Parser;

#[derive(Parser)]
#[grammar = "filter/syntax.pest"]
pub struct FilterParser;