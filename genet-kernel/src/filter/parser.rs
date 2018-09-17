use combine::{
    between, choice,
    combinator::{parser, recognize},
    from_str, many, many1, not_followed_by,
    parser::char::{alpha_num, digit, hex_digit, letter, oct_digit, spaces, string},
    token, try, Parser,
};
use combine_language;
use filter::{ast::Expr, variant::Variant};
use genet_abi::token::Token;
use num_bigint::BigInt;
use num_traits::Num;

fn unsigned_bin<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0b"))
        .with(many1(token('0').or(token('1'))))
        .map(|s: String| BigInt::from_str_radix(&s, 2).unwrap())
}

fn unsigned_oct<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0o"))
        .with(many1(oct_digit()))
        .map(|s: String| BigInt::from_str_radix(&s, 8).unwrap())
}

fn unsigned_hex<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string("0x"))
        .with(many1(hex_digit()))
        .map(|s: String| BigInt::from_str_radix(&s, 16).unwrap())
}

fn unsigned_dec<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    many1(digit()).map(|s: String| BigInt::from_str_radix(&s, 10).unwrap())
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
        try(boolean().map(|v| Variant::Bool(v))),
        try(float().map(|v| Variant::Float64(v))),
        try(number().map(|v| Variant::BigInt(v).shrink())),
    )).map(|v| Expr::Literal(v))
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
    spaces().with(
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
    )
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
    combine_language::expression_parser(unary(), op_parser, op)
}

#[cfg(test)]
mod tests {
    use super::*;
    use filter::{ast::Expr, variant::Variant};
    use genet_abi::token::Token;

    #[test]
    fn literal() {
        assert_eq!(
            expression().parse("true"),
            Ok((Expr::Literal(Variant::Bool(true)), ""))
        );
        assert_eq!(
            expression().parse("truely"),
            Ok((Expr::Token(Token::from("truely")), ""))
        );
        assert_eq!(
            expression().parse("false"),
            Ok((Expr::Literal(Variant::Bool(false)), ""))
        );
        assert_eq!(
            expression().parse("falsely"),
            Ok((Expr::Token(Token::from("falsely")), ""))
        );
        assert_eq!(
            expression().parse("false.false"),
            Ok((Expr::Token(Token::from("false.false")), ""))
        );
        assert_eq!(
            expression().parse("0"),
            Ok((Expr::Literal(Variant::UInt64(0)), ""))
        );
        assert_eq!(
            expression().parse("0.0"),
            Ok((Expr::Literal(Variant::Float64(0.0)), ""))
        );
        assert_eq!(
            expression().parse("1234"),
            Ok((Expr::Literal(Variant::UInt64(1234)), ""))
        );
        assert_eq!(
            expression().parse("0b110110"),
            Ok((Expr::Literal(Variant::UInt64(54)), ""))
        );
        assert_eq!(
            expression().parse("0o776503"),
            Ok((Expr::Literal(Variant::UInt64(261443)), ""))
        );
        assert_eq!(
            expression().parse("0xff5678"),
            Ok((Expr::Literal(Variant::UInt64(16733816)), ""))
        );
        assert_eq!(
            expression().parse("(((((0xff5678)))))"),
            Ok((Expr::Literal(Variant::UInt64(16733816)), ""))
        );
    }
}
