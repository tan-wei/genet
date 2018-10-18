use filter::{ast::Expr, variant::VariantExt};
use genet_abi::variant::Variant;
use genet_abi::token::Token;

pub fn unparse_attr(_typ: &Token, var: Variant) -> Expr {
    Expr::Literal(var)
}

pub fn unparse(expr: &Expr) -> String {
    match expr {
        Expr::Literal(var) => var.to_string(),
        Expr::Token(t) => t.to_string(),
        Expr::Macro(expr) => format!("@{}", expr),
        Expr::CmpEq(lhs, rhs) => match (lhs.as_ref(), rhs.as_ref()) {
            (lhs, &Expr::Literal(Variant::Bool(true))) => unparse(lhs),
            (lhs, &Expr::Literal(Variant::Bool(false))) => format!("!{}", unparse(lhs)),
            (&Expr::Literal(Variant::Bool(true)), rhs) => unparse(rhs),
            (&Expr::Literal(Variant::Bool(false)), rhs) => format!("!{}", unparse(rhs)),
            (lhs, rhs) => format!("{} == {}", unparse(lhs), unparse(rhs)),
        },
        Expr::CmpNotEq(lhs, rhs) => match (lhs.as_ref(), rhs.as_ref()) {
            (lhs, &Expr::Literal(Variant::Bool(false))) => unparse(lhs),
            (lhs, &Expr::Literal(Variant::Bool(true))) => format!("!{}", unparse(lhs)),
            (&Expr::Literal(Variant::Bool(false)), rhs) => unparse(rhs),
            (&Expr::Literal(Variant::Bool(true)), rhs) => format!("!{}", unparse(rhs)),
            (lhs, rhs) => format!("{} != {}", unparse(lhs), unparse(rhs)),
        },
        Expr::CmpLt(lhs, rhs) => format!("{} < {}", unparse(lhs), unparse(rhs)),
        Expr::CmpGt(lhs, rhs) => format!("{} > {}", unparse(lhs), unparse(rhs)),
        Expr::CmpLte(lhs, rhs) => format!("{} <= {}", unparse(lhs), unparse(rhs)),
        Expr::CmpGte(lhs, rhs) => format!("{} >= {}", unparse(lhs), unparse(rhs)),
        Expr::LogicalAnd(lhs, rhs) => format!("{} && {}", unparse(lhs), unparse(rhs)),
        Expr::LogicalOr(lhs, rhs) => format!("{} || {}", unparse(lhs), unparse(rhs)),
        Expr::LogicalNegation(expr) => format!("!{}", unparse(expr)),
        Expr::UnaryPlus(expr) => format!("+{}", unparse(expr)),
        Expr::UnaryNegation(expr) => format!("-{}", unparse(expr)),
    }
}
