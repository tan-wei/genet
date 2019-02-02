use crate::{ast::Expr, variant::VariantExt};
use arrayref::array_ref;
use genet_abi::variant::Variant;
use hwaddr::HwAddr;
use std::net::{Ipv4Addr, Ipv6Addr};

pub fn unparse_attr(typ: &str, var: &Variant) -> Expr {
    match (typ, var) {
        ("@ipv4:addr", Variant::Bytes(b)) => {
            if b.len() == 4 {
                return Expr::Macro(Ipv4Addr::from(*array_ref![b, 0, 4]).to_string());
            }
        }
        ("@ipv6:addr", Variant::Bytes(b)) => {
            if b.len() == 16 {
                return Expr::Macro(Ipv6Addr::from(*array_ref![b, 0, 16]).to_string());
            }
        }
        ("@eth:mac", Variant::Bytes(b)) => {
            if b.len() == 6 {
                return Expr::Macro(HwAddr::from(*array_ref![b, 0, 6]).to_string());
            }
        }
        _ => {}
    }
    Expr::Literal(var.clone())
}

pub fn unparse(expr: &Expr) -> String {
    match expr {
        Expr::Literal(var) => var.to_string(),
        Expr::Token(t) => "[t]".to_string(),
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
