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
use genet_abi::variant::Variant;
use num_bigint::BigInt;
use num_traits::{Num, ToPrimitive, Zero};

fn unsigned_bin<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string_cmp("0b", |l, r| l.eq_ignore_ascii_case(&r)))
        .with(many1(token('0').or(token('1'))))
        .map(|s: String| BigInt::from_str_radix(&s, 2).unwrap())
}

fn unsigned_oct<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string_cmp("0o", |l, r| l.eq_ignore_ascii_case(&r)))
        .with(many1(oct_digit()))
        .map(|s: String| BigInt::from_str_radix(&s, 8).unwrap())
}

fn unsigned_hex<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    try(string_cmp("0x", |l, r| l.eq_ignore_ascii_case(&r)))
        .with(many1(hex_digit()))
        .map(|s: String| BigInt::from_str_radix(&s, 16).unwrap())
}

fn unsigned_dec<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    many1(digit()).map(|s: String| BigInt::from_str_radix(&s, 10).unwrap())
}

fn signed_integer<'a>() -> impl Parser<Input = &'a str, Output = BigInt> {
    (
        optional(token('-')),
        choice((
            unsigned_hex(),
            unsigned_oct(),
            unsigned_bin(),
            unsigned_dec(),
        )),
    )
        .map(|(s, v)| if s.is_some() { BigInt::zero() - v } else { v })
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
    (token('.'), chunk()).map(|v: (char, String)| v.0.to_string() + &v.1)
}

fn identifier<'a>() -> impl Parser<Input = &'a str, Output = String> {
    (chunk(), many(member())).map(|v: (String, String)| v.0 + &v.1)
}

fn literal<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    choice((
        boolean().map(|v| Variant::Bool(v)),
        signed_integer().map(|v| {
            if let Some(i) = v.to_u64() {
                Variant::UInt64(i)
            } else if let Some(i) = v.to_i64() {
                Variant::Int64(i)
            } else {
                let (_, b) = v.to_bytes_be();
                Variant::Buffer(b.into_boxed_slice())
            }
        }),
    )).map(|v| Expression::Literal(v))
}

fn term<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    spaces()
        .with(
            between(
                token('('),
                token(')'),
                parser(|input| expression().parse_stream(input)),
            ).or(literal()),
        ).skip(spaces())
}

fn unary<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    (
        many::<Vec<_>, _>(choice([string("!"), string("~"), string("+"), string("-")])),
        term(),
    )
        .map(|(ops, exp)| {
            ops.into_iter().rfold(exp, |acc, op| match op {
                "!" => Expression::LogicalNegation(Box::new(acc)),
                "~" => Expression::BitwiseNot(Box::new(acc)),
                "+" => Expression::UnaryPlus(Box::new(acc)),
                "-" => Expression::UnaryNegation(Box::new(acc)),
                _ => unreachable!(),
            })
        })
}

fn expression<'a>() -> impl Parser<Input = &'a str, Output = Expression> {
    let op_parser = try(choice([
        string("*"),
        string("/"),
        string("%"),
        string("+"),
        string("-"),
        string("<<"),
        string(">>"),
        string(">>>"),
        string("<"),
        string("<="),
        string(">"),
        string(">="),
        string("=="),
        string("!="),
        string("&"),
        string("^"),
        string("|"),
        string("&&"),
        string("||"),
    ])).map(|op| {
        let prec = match op {
            "*" | "/" | "%" => 14,
            "+" | "-" => 13,
            "<<" | ">>" | ">>>" => 12,
            "<" | "<=" | ">" | ">=" => 11,
            "==" | "!=" => 10,
            "&" => 9,
            "^" => 8,
            "|" => 7,
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
        "*" => Expression::Multiplication(Box::new(l), Box::new(r)),
        "/" => Expression::Division(Box::new(l), Box::new(r)),
        "%" => Expression::Reminder(Box::new(l), Box::new(r)),
        "+" => Expression::Addition(Box::new(l), Box::new(r)),
        "-" => Expression::Subtraction(Box::new(l), Box::new(r)),
        "<<" => Expression::LeftShift(Box::new(l), Box::new(r)),
        ">>" => Expression::RightShit(Box::new(l), Box::new(r)),
        ">>>" => Expression::UnsignedRightShit(Box::new(l), Box::new(r)),
        "==" => Expression::CmpEq(Box::new(l), Box::new(r)),
        "!=" => Expression::CmpNotEq(Box::new(l), Box::new(r)),
        "<" => Expression::CmpLt(Box::new(l), Box::new(r)),
        ">" => Expression::CmpGt(Box::new(l), Box::new(r)),
        "<=" => Expression::CmpLte(Box::new(l), Box::new(r)),
        ">=" => Expression::CmpGte(Box::new(l), Box::new(r)),
        "&" => Expression::BitwiseAnd(Box::new(l), Box::new(r)),
        "|" => Expression::BitwiseOr(Box::new(l), Box::new(r)),
        "^" => Expression::BitwiseXor(Box::new(l), Box::new(r)),
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
        let fi = "5 - -8 * (4 + 4) * 2 + 1";
        println!("{:?}", expression().parse(fi));
        println!("{:?}", expression().parse(fi).unwrap().0.fold());
    }
}
