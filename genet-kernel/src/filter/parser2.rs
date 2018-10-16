use filter::{ast::Expr, variant::VariantExt};
use genet_abi::{token::Token, variant::Variant};
use num_bigint::BigInt;
use num_traits::Num;
use pest::{iterators::Pair, Parser};

#[derive(Parser)]
#[grammar = "filter/syntax.pest"]
pub struct FilterParser;

fn parse(filter: &str) -> Expr {
    let result = FilterParser::parse(Rule::filter, filter);
    let expr = result.unwrap().next().unwrap().into_inner().next().unwrap();
    consume_expr(expr)
}

fn consume_expr(pair: Pair<Rule>) -> Expr {
    let next = pair.into_inner().next().unwrap();
    match next.as_rule() {
        Rule::primary => consume_primary(next),
        _ => Expr::Literal(Variant::Nil),
    }
}

fn consume_primary(pair: Pair<Rule>) -> Expr {
    let mut result = None;
    for item in pair.into_inner().rev() {
        result = Some(match item.as_rule() {
            Rule::expression => consume_expr(item),
            Rule::op_unary_plus => Expr::UnaryPlus(Box::new(result.take().unwrap())),
            Rule::op_unary_negation => Expr::UnaryNegation(Box::new(result.take().unwrap())),
            Rule::op_logical_negation => Expr::LogicalNegation(Box::new(result.take().unwrap())),
            Rule::bin_integer => {
                let v = BigInt::from_str_radix(&item.as_str()[2..], 2).unwrap();
                Expr::Literal(Variant::BigInt(v.to_signed_bytes_be().into_boxed_slice()).shrink())
            }
            Rule::oct_integer => {
                let v = BigInt::from_str_radix(&item.as_str()[2..], 8).unwrap();
                Expr::Literal(Variant::BigInt(v.to_signed_bytes_be().into_boxed_slice()).shrink())
            }
            Rule::hex_integer => {
                let v = BigInt::from_str_radix(&item.as_str()[2..], 16).unwrap();
                Expr::Literal(Variant::BigInt(v.to_signed_bytes_be().into_boxed_slice()).shrink())
            }
            Rule::dec_integer => {
                let v = BigInt::from_str_radix(item.as_str(), 10).unwrap();
                Expr::Literal(Variant::BigInt(v.to_signed_bytes_be().into_boxed_slice()).shrink())
            }
            _ => Expr::Literal(Variant::Bool(false)),
        });
    }
    result.unwrap()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn literal() {
        println!("{:?}", parse("- (0b1010)"));
    }
}
