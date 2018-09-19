use combine::{
    between, choice,
    combinator::{parser, recognize},
    eof, from_str, many, many1, not_followed_by,
    parser::char::{alpha_num, digit, hex_digit, letter, oct_digit, spaces, string},
    skip_many, token, try, Parser,
};
use combine_language;
use filter::{ast::Expr, variant::Variant};
use genet_abi::token::Token;
use num_bigint::BigInt;
use num_traits::Num;

fn unsigned_bin<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0b"))
        .with(many1(token('0').or(token('1').skip(skip_many(token('_'))))))
        .map(|s: String| BigInt::from_str_radix(&s, 2).unwrap())
}

fn unsigned_oct<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0o"))
        .with(many1(oct_digit().skip(skip_many(token('_')))))
        .map(|s: String| BigInt::from_str_radix(&s, 8).unwrap())
}

fn unsigned_hex<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0x"))
        .with(many1(hex_digit().skip(skip_many(token('_')))))
        .map(|s: String| BigInt::from_str_radix(&s, 16).unwrap())
}

fn unsigned_dec<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    many1(digit().skip(skip_many(token('_'))))
        .map(|s: String| BigInt::from_str_radix(&s, 10).unwrap())
}

fn float<'a>() -> impl Parser<Input = &'a str, Output = f64> {
    from_str(recognize::<String, _>((
        many1::<String, _>(digit()),
        token('.'),
        many1::<String, _>(digit()),
    )))
}

fn number<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    choice((
        unsigned_hex(),
        unsigned_oct(),
        unsigned_bin(),
        unsigned_dec(),
    ))
}

fn boolean<'a>() -> impl Parser<Input = &'a str, Output = bool> {
    (string("true")
        .map(|_| true)
        .or(string("false").map(|_| false))).skip(not_followed_by(alpha_num().or(token('.'))))
}

fn chunk<'a>() -> impl Parser<Input = &'a str, Output = String> {
    (
        many1(letter().or(token('_'))),
        many(alpha_num().or(token('_'))),
    )
        .map(|v: (String, String)| v.0 + &v.1)
}

fn member<'a>() -> impl Parser<Input = &'a str, Output = String> {
    token('.').with(chunk())
}

fn identifier<'a>() -> impl Parser<Input = &'a str, Output = Expr> {
    (chunk(), many::<Vec<String>, _>(member())).map(|v| {
        let mut id = v.0;
        for m in v.1 {
            id += ".";
            id += &m;
        }
        Expr::Token(Token::from(id))
    })
}

fn literal<'a>() -> impl Parser<Input = &'a str, Output = Expr> {
    choice((
        try(boolean().map(Variant::Bool)),
        try(float().map(Variant::Float64)),
        try(number().map(|v| Variant::BigInt(v).shrink())),
    )).map(Expr::Literal)
}

fn term<'a>() -> impl Parser<Input = &'a str, Output = Expr> {
    spaces()
        .with(
            between(
                token('('),
                token(')'),
                parser(|input| expression().parse_stream(input)),
            ).or(literal())
            .or(identifier()),
        ).skip(spaces())
}

fn unary<'a>() -> impl Parser<Input = &'a str, Output = Expr> {
    spaces()
        .with(
            (
                many::<Vec<_>, _>(choice([string("!"), string("+"), string("-")])),
                term(),
            )
                .map(|(ops, exp)| {
                    ops.into_iter().rfold(exp, |acc, op| match op {
                        "!" => Expr::LogicalNegation(Box::new(acc)),
                        "+" => Expr::UnaryPlus(Box::new(acc)),
                        "-" => Expr::UnaryNegation(Box::new(acc)),
                        _ => unreachable!(),
                    })
                }),
        ).skip(spaces())
}

fn op(l: Expr, op: &'static str, r: Expr) -> Expr {
    match op {
        "==" => Expr::CmpEq(Box::new(l), Box::new(r)),
        "!=" => Expr::CmpNotEq(Box::new(l), Box::new(r)),
        "<" => Expr::CmpLt(Box::new(l), Box::new(r)),
        ">" => Expr::CmpGt(Box::new(l), Box::new(r)),
        "<=" => Expr::CmpLte(Box::new(l), Box::new(r)),
        ">=" => Expr::CmpGte(Box::new(l), Box::new(r)),
        "&&" => Expr::LogicalAnd(Box::new(l), Box::new(r)),
        "||" => Expr::LogicalOr(Box::new(l), Box::new(r)),
        _ => unreachable!(),
    }
}

pub fn expression<'a>() -> impl Parser<Input = &'a str, Output = Expr> {
    let op_parser = choice([
        try(string("<=")),
        try(string("<")),
        try(string(">=")),
        try(string(">")),
        try(string("==")),
        try(string("!=")),
        try(string("&&")),
        try(string("||")),
    ]).map(|op| {
        let prec = match op {
            "<" | "<=" | ">" | ">=" => 11,
            "==" | "!=" => 10,
            "&&" => 6,
            "||" => 5,
            _ => unreachable!(),
        };
        (
            op,
            combine_language::Assoc {
                precedence: prec,
                fixity: combine_language::Fixity::Left,
            },
        )
    }).skip(spaces());
    combine_language::expression_parser(unary(), op_parser, op).skip((spaces(), eof()))
}

#[cfg(test)]
mod tests {
    use super::*;
    use filter::{ast::Expr::*, variant::Variant};
    use genet_abi::token::Token;
    use num_bigint::BigInt;

    #[test]
    fn literal() {
        assert_eq!(
            expression().parse("true"),
            Ok((Literal(Variant::Bool(true)), ""))
        );
        assert_eq!(
            expression().parse("truely"),
            Ok((Token(Token::from("truely")), ""))
        );
        assert_eq!(
            expression().parse("false"),
            Ok((Literal(Variant::Bool(false)), ""))
        );
        assert_eq!(
            expression().parse("falsely"),
            Ok((Token(Token::from("falsely")), ""))
        );
        assert_eq!(
            expression().parse("false.false"),
            Ok((Token(Token::from("false.false")), ""))
        );
        assert_eq!(
            expression().parse("0"),
            Ok((Literal(Variant::UInt64(0)), ""))
        );
        assert_eq!(
            expression().parse("0.0"),
            Ok((Literal(Variant::Float64(0.0)), ""))
        );
        assert_eq!(
            expression().parse("1234"),
            Ok((Literal(Variant::UInt64(1234)), ""))
        );
        assert_eq!(
            expression().parse("1_2__3___4"),
            Ok((Literal(Variant::UInt64(1234)), ""))
        );
        assert_eq!(
            expression().parse("0b11_0110"),
            Ok((Literal(Variant::UInt64(54)), ""))
        );
        assert_eq!(
            expression().parse("0o77_6503"),
            Ok((Literal(Variant::UInt64(261443)), ""))
        );
        assert_eq!(
            expression().parse("0xff_56_78"),
            Ok((Literal(Variant::UInt64(16733816)), ""))
        );
        assert_eq!(
            expression().parse("9999999999999999999999999"),
            Ok((
                Literal(Variant::BigInt(
                    BigInt::from_str_radix("9999999999999999999999999", 10).unwrap()
                )),
                ""
            ))
        );
        assert_eq!(
            expression().parse("0b1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"),
            Ok((
                Literal(Variant::BigInt(
                    BigInt::from_str_radix(
                        "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
                        2
                    ).unwrap()
                )),
                ""
            ))
        );
        assert_eq!(
            expression().parse("0o7777777777777777777777777777777777777"),
            Ok((
                Literal(Variant::BigInt(
                    BigInt::from_str_radix("7777777777777777777777777777777777777", 8).unwrap()
                )),
                ""
            ))
        );
        assert_eq!(
            expression().parse("0xffffffffffffffffffffffffffff"),
            Ok((
                Literal(Variant::BigInt(
                    BigInt::from_str_radix("ffffffffffffffffffffffffffff", 16).unwrap()
                )),
                ""
            ))
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            expression().parse("0xff5678"),
            Ok((Literal(Variant::UInt64(16733816)), ""))
        );
    }

    #[test]
    fn unary() {
        assert_eq!(
            expression().parse("+-!123"),
            Ok((
                UnaryPlus(Box::new(UnaryNegation(Box::new(LogicalNegation(
                    Box::new(Literal(Variant::UInt64(123)))
                ))))),
                ""
            ))
        );
    }

    #[test]
    fn binary() {
        assert_eq!(
            expression().parse("12.5 == 0xff"),
            Ok((
                CmpEq(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 != 0xff"),
            Ok((
                CmpNotEq(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 <= 0xff"),
            Ok((
                CmpLte(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 < 0xff"),
            Ok((
                CmpLt(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 >= 0xff"),
            Ok((
                CmpGte(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 > 0xff"),
            Ok((
                CmpGt(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 && 0xff"),
            Ok((
                LogicalAnd(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );

        assert_eq!(
            expression().parse("12.5 || 0xff"),
            Ok((
                LogicalOr(
                    Box::new(Literal(Variant::Float64(12.5))),
                    Box::new(Literal(Variant::UInt64(255)))
                ),
                ""
            ))
        );
    }

    #[test]
    fn error() {
        assert!(expression().parse("| 12.5").is_err());
        assert!(expression().parse("== 12.5").is_err());
        assert!(expression().parse("!= 12.5").is_err());
        assert!(expression().parse("&& 12.5").is_err());
        assert!(expression().parse("|| 12.5").is_err());
        assert!(expression().parse("<= 12.5").is_err());
        assert!(expression().parse(">= 12.5").is_err());
        assert!(expression().parse("< 12.5").is_err());
        assert!(expression().parse("> 12.5").is_err());
    }
}
