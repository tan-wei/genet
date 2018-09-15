use ast::Expression;
use combine::{
    between, choice,
    combinator::parser,
    eof, many, many1, optional,
    parser::{
        self,
        char::{alpha_num, digit, hex_digit, letter, oct_digit, spaces, string, string_cmp},
        combinator::recognize,
    },
    token, try, ParseResult, Parser, Stream,
};
use combine_language;
use genet_abi::token::Token;
use num_bigint::BigInt;
use num_traits::{Num, ToPrimitive, Zero};
use variant::Variant;

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

fn unsigned_integer<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    choice((
        unsigned_hex(),
        unsigned_oct(),
        unsigned_bin(),
        unsigned_dec(),
    ))
}

fn boolean<'a>() -> impl Parser<Input = &'a str, Output = bool> {
    string("true")
        .map(|_| true)
        .or(string("false").map(|_| false))
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

fn identifier<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    (chunk(), many::<Vec<String>, _>(member())).map(|v| {
        let mut id = v.0;
        for m in v.1 {
            id += &m;
        }
        Expression::Token(Token::from(id))
    })
}

fn literal<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    choice((
        boolean().map(|v| Variant::Bool(v)),
        unsigned_integer().map(|v| Variant::BigInt(v).shrink()),
    )).map(|v| Expression::Literal(v))
}

fn term<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    spaces()
        .with(
            between(
                token('('),
                token(')'),
                parser(|input| expression().parse_stream(input)),
            ).or(identifier())
            .or(literal()),
        ).skip(spaces())
}

fn unary<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    spaces().with(
        (
            many::<Vec<_>, _>(choice([string("!"), string("~"), string("+"), string("-")])),
            term(),
        )
            .map(|(ops, exp)| {
                ops.into_iter().rfold(exp, |acc, op| match op {
                    "!" => Expression::LogicalNegation(Box::new(acc)),
                    _ => unreachable!(),
                })
            }),
    )
}

fn expression<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    let op_parser = try(choice([
        string("<"),
        string("<="),
        string(">"),
        string(">="),
        string("=="),
        string("!="),
        string("&&"),
        string("||"),
    ])).map(|op| {
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

fn op(l: Expression, op: &'static str, r: Expression) -> Expression {
    match op {
        "==" => Expression::CmpEq(Box::new(l), Box::new(r)),
        "!=" => Expression::CmpNotEq(Box::new(l), Box::new(r)),
        "<" => Expression::CmpLt(Box::new(l), Box::new(r)),
        ">" => Expression::CmpGt(Box::new(l), Box::new(r)),
        "<=" => Expression::CmpLte(Box::new(l), Box::new(r)),
        ">=" => Expression::CmpGte(Box::new(l), Box::new(r)),
        "&&" => Expression::LogicalAnd(Box::new(l), Box::new(r)),
        "||" => Expression::LogicalOr(Box::new(l), Box::new(r)),
        _ => unreachable!(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use ast::Context;

    #[test]
    fn decode() {
        let ctx = Context {};
        let fi = "tcp && 100 == tcp";
        println!("{:?}", expression().parse(fi));
    }
}
