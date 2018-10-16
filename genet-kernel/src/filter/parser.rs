use filter::{ast::Expr, variant::VariantExt};
use genet_abi::{token::Token, variant::Variant};
use num_bigint::BigInt;
use num_traits::Num;
use pest::{
    error::Error,
    iterators::Pair,
    prec_climber::{Assoc, Operator, PrecClimber},
    Parser,
};

#[derive(Parser)]
#[grammar = "filter/syntax.pest"]
pub struct FilterParser;

pub fn parse(filter: &str) -> Result<Expr, Error<Rule>> {
    let result = FilterParser::parse(Rule::filter, filter);
    match result {
        Ok(mut expr) => Ok(consume_expr(
            expr.next().unwrap().into_inner().next().unwrap(),
        )),
        Err(e) => Err(e),
    }
}

fn consume_expr(pair: Pair<Rule>) -> Expr {
    let climber = PrecClimber::new(vec![
        Operator::new(Rule::op_lt, Assoc::Left)
            | Operator::new(Rule::op_lte, Assoc::Left)
            | Operator::new(Rule::op_gt, Assoc::Left)
            | Operator::new(Rule::op_gte, Assoc::Left),
        Operator::new(Rule::op_eq, Assoc::Left) | Operator::new(Rule::op_ne, Assoc::Left),
        Operator::new(Rule::op_logical_and, Assoc::Left),
        Operator::new(Rule::op_logical_or, Assoc::Left),
    ]);
    let primary = |pair: Pair<Rule>| match pair.as_rule() {
        Rule::primary => consume_primary(pair),
        _ => Expr::Literal(Variant::Nil),
    };
    let infix = |lhs: Expr, op: Pair<Rule>, rhs: Expr| -> Expr {
        match op.as_rule() {
            Rule::op_lt => Expr::CmpLt(Box::new(lhs), Box::new(rhs)),
            Rule::op_lte => Expr::CmpLte(Box::new(lhs), Box::new(rhs)),
            Rule::op_gt => Expr::CmpGt(Box::new(lhs), Box::new(rhs)),
            Rule::op_gte => Expr::CmpGte(Box::new(lhs), Box::new(rhs)),
            Rule::op_eq => Expr::CmpEq(Box::new(lhs), Box::new(rhs)),
            Rule::op_ne => Expr::CmpNotEq(Box::new(lhs), Box::new(rhs)),
            Rule::op_logical_and => Expr::LogicalAnd(Box::new(lhs), Box::new(rhs)),
            Rule::op_logical_or => Expr::LogicalOr(Box::new(lhs), Box::new(rhs)),
            _ => Expr::Literal(Variant::Nil),
        }
    };
    climber.climb(pair.into_inner(), primary, infix)
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
            Rule::float => Expr::Literal(Variant::Float64(item.as_str().parse().unwrap())),
            Rule::boolean => Expr::Literal(Variant::Bool(item.as_str() == "true")),
            Rule::member => Expr::Token(Token::from(item.as_str())),
            _ => Expr::Literal(Variant::Nil),
        });
    }
    result.unwrap()
}

#[cfg(test)]
mod tests {
    use super::*;
    use filter::ast::Expr::*;
    use genet_abi::{token::Token, variant::Variant};
    use num_bigint::BigInt;

    #[test]
    fn literal() {
        assert_eq!(parse("true"), Ok(Literal(Variant::Bool(true))));
        assert_eq!(parse("truely"), Ok(Token(Token::from("truely"))));
        assert_eq!(parse("false"), Ok(Literal(Variant::Bool(false))));
        assert_eq!(parse("falsely"), Ok(Token(Token::from("falsely"))));
        assert_eq!(parse("false.false"), Ok(Token(Token::from("false.false"))));
        assert_eq!(parse("0"), Ok(Literal(Variant::UInt64(0))));
        assert_eq!(parse("0.0"), Ok(Literal(Variant::Float64(0.0))));
        assert_eq!(parse("1234"), Ok(Literal(Variant::UInt64(1234))));
        assert_eq!(parse("1234"), Ok(Literal(Variant::UInt64(1234))));
        assert_eq!(parse("0b110110"), Ok(Literal(Variant::UInt64(54))));
        assert_eq!(parse("0o776503"), Ok(Literal(Variant::UInt64(261_443))));
        assert_eq!(parse("0xff5678"), Ok(Literal(Variant::UInt64(16_733_816))));
        assert_eq!(
            parse("9999999999999999999999999"),
            Ok(Literal(Variant::BigInt(
                BigInt::from_str_radix("9999999999999999999999999", 10)
                    .unwrap()
                    .to_signed_bytes_be()
                    .into_boxed_slice()
            )))
        );
        assert_eq!(
            parse("0b1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"),
            Ok(
                Literal(Variant::BigInt(
                    BigInt::from_str_radix(
                        "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
                        2
                    ).unwrap().to_signed_bytes_be().into_boxed_slice()
                ))
            )
        );
        assert_eq!(
            parse("0o7777777777777777777777777777777777777"),
            Ok(Literal(Variant::BigInt(
                BigInt::from_str_radix("7777777777777777777777777777777777777", 8)
                    .unwrap()
                    .to_signed_bytes_be()
                    .into_boxed_slice()
            )))
        );
        assert_eq!(
            parse("0xffffffffffffffffffffffffffff"),
            Ok(Literal(Variant::BigInt(
                BigInt::from_str_radix("ffffffffffffffffffffffffffff", 16)
                    .unwrap()
                    .to_signed_bytes_be()
                    .into_boxed_slice()
            )))
        );
    }

    #[test]
    fn group() {
        assert_eq!(parse("0xff5678"), Ok(Literal(Variant::UInt64(16_733_816))));
    }

    #[test]
    fn unary() {
        assert_eq!(
            parse("+-!123"),
            Ok(UnaryPlus(Box::new(UnaryNegation(Box::new(
                LogicalNegation(Box::new(Literal(Variant::UInt64(123))))
            )))))
        );
    }

    #[test]
    fn binary() {
        assert_eq!(
            parse("12.5 == 0xff"),
            Ok(CmpEq(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 != 0xff"),
            Ok(CmpNotEq(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 <= 0xff"),
            Ok(CmpLte(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 < 0xff"),
            Ok(CmpLt(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 >= 0xff"),
            Ok(CmpGte(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 > 0xff"),
            Ok(CmpGt(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 && 0xff"),
            Ok(LogicalAnd(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );

        assert_eq!(
            parse("12.5 || 0xff"),
            Ok(LogicalOr(
                Box::new(Literal(Variant::Float64(12.5))),
                Box::new(Literal(Variant::UInt64(255)))
            ))
        );
    }

    #[test]
    fn error() {
        assert!(parse("| 12.5").is_err());
        assert!(parse("== 12.5").is_err());
        assert!(parse("!= 12.5").is_err());
        assert!(parse("&& 12.5").is_err());
        assert!(parse("|| 12.5").is_err());
        assert!(parse("<= 12.5").is_err());
        assert!(parse(">= 12.5").is_err());
        assert!(parse("< 12.5").is_err());
        assert!(parse("> 12.5").is_err());
    }
}
