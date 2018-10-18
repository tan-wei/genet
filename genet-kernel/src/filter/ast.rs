use filter::{context::Context, variant::VariantExt};
use genet_abi::{token::Token, variant::Variant};

#[derive(PartialEq, Clone, Debug)]
pub enum Expr {
    Literal(Variant),
    Token(Token),
    Macro(String),
    CmpEq(Box<Expr>, Box<Expr>),
    CmpNotEq(Box<Expr>, Box<Expr>),
    CmpLt(Box<Expr>, Box<Expr>),
    CmpGt(Box<Expr>, Box<Expr>),
    CmpLte(Box<Expr>, Box<Expr>),
    CmpGte(Box<Expr>, Box<Expr>),
    LogicalAnd(Box<Expr>, Box<Expr>),
    LogicalOr(Box<Expr>, Box<Expr>),
    LogicalNegation(Box<Expr>),
    UnaryPlus(Box<Expr>),
    UnaryNegation(Box<Expr>),
}

impl Expr {
    pub fn eval(&self, ctx: &Context) -> Variant {
        match self {
            Expr::Literal(v) => v.clone(),
            Expr::CmpEq(l, r) => Variant::Bool(l.eval(ctx).op_eq(&r.eval(ctx))),
            Expr::CmpNotEq(l, r) => Variant::Bool(!l.eval(ctx).op_eq(&r.eval(ctx))),
            Expr::CmpLt(l, r) => Variant::Bool(l.eval(ctx).op_lt(&r.eval(ctx))),
            Expr::CmpGt(l, r) => Variant::Bool(l.eval(ctx).op_gt(&r.eval(ctx))),
            Expr::CmpLte(l, r) => Variant::Bool(l.eval(ctx).op_lte(&r.eval(ctx))),
            Expr::CmpGte(l, r) => Variant::Bool(l.eval(ctx).op_gte(&r.eval(ctx))),
            Expr::LogicalAnd(l, r) => {
                Variant::Bool(l.eval(ctx).is_truthy() && r.eval(ctx).is_truthy())
            }
            Expr::LogicalOr(l, r) => {
                Variant::Bool(l.eval(ctx).is_truthy() || r.eval(ctx).is_truthy())
            }
            Expr::LogicalNegation(v) => Variant::Bool(!v.eval(ctx).is_truthy()),
            Expr::UnaryPlus(v) => v.eval(ctx).op_unary_plus(),
            Expr::UnaryNegation(v) => v.eval(ctx).op_unary_negation(),
            Expr::Token(t) => {
                for layer in ctx.frame().layers().iter().rev() {
                    if layer.id() == *t {
                        return Variant::Bool(true);
                    }
                    if let Some(attr) = layer
                        .headers()
                        .iter()
                        .chain(layer.attrs().iter())
                        .find(|a| a.id() == *t)
                    {
                        if let Ok(val) = attr.try_get(layer) {
                            return val;
                        }
                    }
                }
                Variant::Nil
            }
            Expr::Macro(_) => Variant::Nil,
        }
    }
}
