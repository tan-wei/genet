use filter::{ast::Expr, variant::VariantExt};

pub fn unparse(expr: &Expr) -> String {
    match expr {
        Expr::Literal(var) => var.to_string(),
        Expr::Token(t) => t.to_string(),
        Expr::CmpEq(lhs, rhs) => format!("{} == {}", unparse(lhs), unparse(rhs)),
        Expr::CmpNotEq(lhs, rhs) => format!("{} != {}", unparse(lhs), unparse(rhs)),
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
